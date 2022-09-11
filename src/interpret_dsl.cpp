#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <any>
#include <bits/ranges_algo.h>
#include <bits/utility.h>
#include <cassert>
#include <charconv>
#include <component/force.h>
#include <component/movement.h>
#include <component/renderers/circle_renderer.h>
#include <cstdint>
#include <cstdlib>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <istream>
#include <logger_ref.h>
#include <memory>
#include <object.h>
#include <object_class.h>
#include <optional>
#include <ostream>
#include <physics.h>
#include <special_object.h>
#include <stdexcept>
#include <string>
#include <tracker.h>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <util/builers.h>
#include <util/vec.h>
#include <variant>
#include <vector>

struct src_location
{
    std::size_t line;
    std::size_t ch;
};

class parse_error : public std::runtime_error
{
    src_location l;
    const char* fix;

public:
    parse_error(const char* msg, const src_location& l, const char* fix = nullptr) : std::runtime_error(msg), l(l), fix(fix)
    {
    }
    constexpr const src_location& loc() { return l; }
    constexpr const char* fix_msg() const { return fix; }
};

class token
{
public:
    enum token_type : int8_t
    {
        TOK_CH = 0,
        TOK_EOF,
        TOK_IDENTIFIER,
        TOK_OPERATOR,
        TOK_SEPERATOR,
        TOK_KW_OBJTYPE,
        TOK_KW_CONTROL,
        TOK_KW_RENDERER,
        TOK_KW_FORCE,
        TOK_LIT_NUMBER,
        TOK_LIT_COLOR,
        TOK_LIT_STR,
    };

private:
    int8_t tok;
    std::variant<std::monostate, char, std::string, double, uint32_t> value;
    src_location loc;

public:
    template <typename T>
    constexpr token(const src_location& loc, token_type t, T&& val) : tok(t), value(val), loc(loc)
    {
    }
    explicit constexpr token(const src_location& loc, char ch) : tok(TOK_CH), value(ch), loc(loc) {}
    explicit constexpr token(const src_location& loc, token_type t) : tok(t), value(std::monostate{}), loc(loc) {}

    constexpr bool is_character() const { return tok == TOK_CH; }
    constexpr token_type type() const { return (token_type)tok; }
    constexpr char ch() const { return std::get<char>(value); }
    constexpr double number() const { return std::get<double>(value); }
    constexpr const std::string& identifier() const { return std::get<std::string>(value); }
    constexpr uint32_t color() const { return std::get<uint32_t>(value); }
    constexpr const std::string& str_lit() const { return std::get<std::string>(value); }
    constexpr const src_location& location() const { return loc; }

    constexpr bool operator==(char c) const { return is_character() && ch() == c; }
};

class parse_context
{
    src_location l{1, 1};
    std::istream& is;
    std::vector<std::string> line_buffer;
    std::string curr_line;
    std::string filename;
    std::vector<std::tuple<std::string, src_location, std::string>> errors;

public:
    constexpr parse_context(std::istream& is, const char* filename) : is(is), filename(filename) {}

    char next_char()
    {
        if (is.eof())
            return EOF;

        char ch = 0;
        is.get(ch);
        curr_line += ch;
        if (ch == '\n')
        {
            l.line++;
            l.ch = 0;
            line_buffer.push_back(curr_line);
            curr_line.clear();
        }

        l.ch++;
        return ch;
    }

    void error(const std::string& message, const src_location& loc, const std::string& fix = "")
    {
        errors.emplace_back(message, loc, fix);
    };

    void dump_errors(std::ostream& os)
    {
        for (const auto& i : errors)
        {
            os << fmt::format("\x1b[31;1;4merror\x1b[0m (at {}:{}:{}): {}\n", filename, std::get<1>(i).line,
                              std::get<1>(i).ch, std::get<0>(i))
               << fmt::format("> {}", line_buffer[std::get<1>(i).line - 1])
               << fmt::format("{: >{}}^\n", "", std::get<1>(i).ch) << fmt::format("potential fix: {}\n", std::get<2>(i));
        }
    }

    constexpr const std::string& src_filename() const { return filename; }
    constexpr const src_location& location() const { return l; }
    bool had_errors() const { return !errors.empty(); }
};

class lexer
{
    char last_ch = ' ';
    parse_context context;

    std::string parse_literal()
    {
        constexpr std::pair<char, char> ESC_SEQUENCES[] = {
            {'a', '\a'}, {'b', '\b'},  {'e', '\x1b'}, {'f', '\f'}, {'n', '\n'},
            {'r', '\r'}, {'\\', '\\'}, {'\'', '\''},  {'"', '"'},
        };

        src_location lit_start = context.location();
        std::string buffer;

        while (true)
        {
            last_ch = context.next_char();
            if (last_ch == EOF)
            {
                context.error("lexer: unexpected EOF while parsing string literal", lit_start);
                return buffer;
            }
            else if (last_ch == '"')
                return buffer;
            else if (last_ch == '\\')
            {
                last_ch = context.next_char();
                if (last_ch == EOF)
                {
                    context.error("lexer: unexpected EOF while parsing escape sequence", lit_start);
                    return buffer;
                }
                else if (last_ch >= '0' && last_ch <= '7')
                {
                    uint16_t val = last_ch - '0';
                    for (int i = 0; i < 2; i++)
                    {
                        last_ch = context.next_char();
                        if (!(last_ch >= '0' && last_ch <= '7'))
                            break;

                        if (last_ch == EOF)
                        {
                            context.error("lexer: unexpected EOF while parsing octal", lit_start);
                            return buffer;
                        }
                        val = (last_ch - '0') + (val << 3);
                    }

                    if (val > INT8_MAX)
                        context.error("lexer: octal literal value overflow", context.location());
                    buffer.push_back((uint8_t)val);
                }
                else if (last_ch == 'x')
                {
                    uint8_t val = 0;
                    for (std::size_t i = 0; i < 2; i++)
                    {
                        last_ch = context.next_char();
                        if (!isxdigit(last_ch))
                            context.error("lexer: invalid character in hex string", context.location());
                        else
                            val = (val << 4) + (isdigit(last_ch) ? (last_ch - '0') : (last_ch - 'a' + 10));
                    }
                    buffer.push_back(val);
                }
                else
                {
                    bool found = false;
                    for (auto i : ESC_SEQUENCES)
                    {
                        if (i.first == last_ch)
                        {
                            buffer.push_back(i.second);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        context.error("lexer: unrecognized character in escape sequence", context.location());
                }
            }
            else if (isprint(last_ch) || last_ch == '\t')
                buffer.push_back(last_ch);
            else
            {
                context.error("lexer: unrecognized character in string literal", context.location());
            }
        }
    }

    constexpr char allowed_as_id(int ch) { return ch == '_' || isalnum(ch); }

public:
    constexpr lexer(parse_context&& context) : context(context) {}
    constexpr parse_context& ctx() { return context; }

    void dump_errors(std::ostream& os) { context.dump_errors(os); }

    token next_token()
    {
        static constexpr std::pair<const char*, token::token_type> KEYWORDS[] = {
            {"objtype", token::TOK_KW_OBJTYPE},
            {"control", token::TOK_KW_CONTROL},
            {"renderer", token::TOK_KW_RENDERER},
            {"force", token::TOK_KW_FORCE},
        };

        static constexpr char OPERATORS[] = {'+', '-', '*', '/', '%', '='};

        while (isspace(last_ch))
            last_ch = context.next_char();

        src_location token_start = context.location();
        if (isalpha(last_ch))
        {
            std::string ident;
            ident = last_ch;
            while (allowed_as_id((last_ch = context.next_char())))
                ident += last_ch;

            for (auto i : KEYWORDS)
                if (ident == i.first)
                    return token(token_start, i.second);
            return token(token_start, token::TOK_IDENTIFIER, ident);
        }
        else if (isdigit(last_ch) || last_ch == '.' || last_ch == '-' || last_ch == '+')
        {
            double num = 0;
            std::string buf;
            do
            {
                buf += last_ch;
                last_ch = context.next_char();
            } while (isdigit(last_ch) || last_ch == '.');

            auto [p, ec] = std::from_chars(buf.data(), buf.data() + buf.size(), num);

            if (ec != std::errc())
            {
                if (buf == ".")
                    return token(token_start, '.');
                else if (buf == "-")
                    return token(token_start, token::TOK_OPERATOR, '-');
                context.error("lexer: unable to parse double", token_start);
            }

            return token(token_start, token::TOK_LIT_NUMBER, num);
        }
        else if (last_ch == '#')
        {
            std::string buf;
            while (true)
            {
                last_ch = context.next_char();
                if (isxdigit(last_ch))
                    buf += last_ch;
                else
                    break;
            }

            if (buf.size() != 6)
            {
                context.error("invalid color literal", token_start);
                return token(token_start, token::TOK_LIT_COLOR, (uint32_t)0);
            }
            uint32_t v = strtol(buf.c_str(), nullptr, 16);
            return token(token_start, token::TOK_LIT_COLOR, v);
        }
        else if (last_ch == ';')
        {
            last_ch = context.next_char();
            return token(token_start, token::TOK_SEPERATOR);
        }
        else if (last_ch == EOF || last_ch == '\0')
            return token(token_start, token::TOK_EOF);
        else if (last_ch == '"')
        {
            auto lit = parse_literal();
            last_ch = context.next_char();
            return token(token_start, token::TOK_LIT_STR, lit);
        }

        for (auto i : OPERATORS)
        {
            if (last_ch == i)
            {
                char curr_ch = last_ch;
                last_ch = context.next_char();
                return token(token_start, token::TOK_OPERATOR, curr_ch);
            }
        }

        int curr_ch = last_ch;
        last_ch = context.next_char();
        return token(token_start, curr_ch);
    }

    bool had_errors() const { return context.had_errors(); }
};

#include <fmt/ranges.h>

struct eval_context
{
    std::unordered_map<std::string, std::any>& vars;
    std::optional<std::any> instance;
    std::vector<std::string> errors;
    phy::object_class_builder* builder;
    phy::physics_space& space;
};

enum class value_category
{
    LVAL,
    RVAL
};

class base_ast
{
    value_category cat;

public:
    constexpr base_ast(value_category cat) : cat(cat) {}
    constexpr value_category category() const { return cat; }
    virtual std::any eval(eval_context&) const = 0;
    virtual void dump_ast(std::ostream& os) const = 0;
    virtual ~base_ast() = default;
};

std::any safe_eval(eval_context& ctx, const std::unique_ptr<base_ast>& ast)
{
    if (ast)
        return ast->eval(ctx);
    return {};
}

template <typename T>
T safe_eval_as(eval_context& ctx, const std::unique_ptr<base_ast>& ast)
{
    return std::any_cast<T>(safe_eval(ctx, ast));
}

template <typename T>
class literal_ast : public base_ast
{
    T value;

public:
    literal_ast(const T& value) : base_ast(value_category::RVAL), value(value) {}

    virtual std::any eval(eval_context&) const override
    {
        if constexpr (std::same_as<T, uint32_t>)
            return sf::Color(value << 8 | 0xff);
        else
            return value;
    }
    virtual void dump_ast(std::ostream& os) const override { os << fmt::format("literal: {}\n", value); }
    virtual ~literal_ast() override = default;
};

using numeric_literal_ast = literal_ast<double>;
using color_literal_ast = literal_ast<uint32_t>;
using string_literal_ast = literal_ast<std::string>;

class variable_expr_ast : public base_ast
{
    std::string name;

public:
    variable_expr_ast(const std::string& name) : base_ast(value_category::LVAL), name(name) {}

    virtual std::any eval(eval_context& ctx) const override
    {
        if (ctx.instance)
        {
            ctx.errors.push_back(fmt::format("cannot find field {} in object", name));
            return {};
        }
        return ctx.vars[name];
    }

    std::any* get(eval_context& ctx) const { return &ctx.vars[name]; };

    virtual void dump_ast(std::ostream& os) const override { os << fmt::format("variable: {}\n", name); }

    virtual ~variable_expr_ast() = default;
};

class binary_expr_ast : public base_ast
{
    char op;
    std::unique_ptr<base_ast> lhs, rhs;

public:
    virtual std::any eval(eval_context& ctx) const override
    {
        if (op == '=')
        {
            if (lhs->category() != value_category::LVAL)
            {
                ctx.errors.push_back("expected lvalue");
                return {};
            }
            std::any* l = dynamic_cast<variable_expr_ast*>(lhs.get())->get(ctx);
            std::any r = safe_eval(ctx, rhs);
            if (r.type() != l->type() && !r.has_value())
            {
                ctx.errors.push_back(
                    fmt::format("type mismatch on assignment: cannot assign {} to {}", r.type().name(), l->type().name()));
                return {};
            }

            *l = r;
            return *l;
        }
        else
        {
            try
            {
                double l = safe_eval_as<double>(ctx, lhs);
                double r = safe_eval_as<double>(ctx, rhs);
                switch (op)
                {
                case '+':
                    return r + l;
                case '-':
                    return r - l;
                case '*':
                    return r * l;
                case '/':
                    return r / l;
                case '%':
                    return std::fmod(r, l);
                default:
                    __builtin_unreachable();
                }
            }
            catch (std::bad_any_cast&)
            {
                ctx.errors.push_back("type mismatch");
                return {};
            }
        }
    }

    virtual void dump_ast(std::ostream& os) const override
    {
        os << fmt::format("binary_expression_ast ({})\n", op);
        os << "lhs:\n";
        lhs->dump_ast(os);
        os << "rhs:\n";
        rhs->dump_ast(os);
    }

    binary_expr_ast(char op, std::unique_ptr<base_ast> lhs, std::unique_ptr<base_ast> rhs)
        : base_ast(value_category::RVAL), op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
    {
    }
};

using arg_list = std::vector<std::unique_ptr<base_ast>>;
using arg_map = std::unordered_map<std::string, std::unique_ptr<base_ast>>;

struct call_fn
{
    const char* name;
    std::vector<std::size_t> type_hash;
    std::any (*invoke)(const std::vector<std::any>&, eval_context&);
    std::size_t this_hash;
};

template <typename... Args>
constexpr std::vector<std::size_t> hashlist_from_type()
{
    return {typeid(Args).hash_code()...};
}

template <typename T, typename... Args, std::any (*fn)(eval_context& ctx, Args...)>
constexpr auto __helper()
{
    return +[](const std::vector<std::any>& args, eval_context&) {
        return fn(std::any_cast<const Args&>(args[std::index_sequence<sizeof...(Args)>::value])...);
    };
}

using dict_type = std::shared_ptr<std::unordered_map<std::string, std::any>>;

template <typename T, auto Fn>
constexpr call_fn make(const char* name)
{
    return [&]<typename... Args>(std::any (*)(eval_context&, Args...)) -> call_fn {
        return {name, hashlist_from_type<Args...>(),
                []<std::size_t... S>(std::integer_sequence<std::size_t, S...>){
                    return +[](const std::vector<std::any>& args, eval_context& c) {
                        return Fn(c, std::any_cast<Args>(args[S])...);
                    };
    }(std::index_sequence_for<Args...>{}),
           std::is_void_v<T> ? 0 : typeid(std::decay_t<T>).hash_code()
};
}
(Fn);
}

static std::unordered_map<std::string, phy::statspec_types> TYPES = {
    {
        "pos",
        phy::statspec_types::POS,
    },
    {
        "vel",
        phy::statspec_types::VEL,
    },
    {
        "momentum",
        phy::statspec_types::MOMENTUM,
    },
    {
        "acc",
        phy::statspec_types::ACC,
    },
    {
        "force",
        phy::statspec_types::FORCE,
    },
    {
        "pos_x",
        phy::statspec_types::POS_X,
    },
    {
        "vel_x",
        phy::statspec_types::VEL_X,
    },
    {
        "momentum_x",
        phy::statspec_types::MOMENTUM_X,
    },
    {
        "acc_x",
        phy::statspec_types::ACC_X,
    },
    {
        "force_x",
        phy::statspec_types::FORCE_X,
    },
    {
        "pos_y",
        phy::statspec_types::POS_Y,
    },
    {
        "vel_y",
        phy::statspec_types::VEL_Y,
    },
    {
        "momentum_y",
        phy::statspec_types::MOMENTUM_Y,
    },
    {
        "acc_y",
        phy::statspec_types::ACC_Y,
    },
    {
        "FORCE_Y",
        phy::statspec_types::FORCE_Y,
    },
    {
        "ke",
        phy::statspec_types::KE,
    },
};

// clang-format off
static call_fn FN_HANDLES[] = {
    make<void, +[](eval_context& ctx, const std::string& name, double mass, const dict_type& e) -> std::any {
        if (!ctx.space.class_exists(name))
        {
            ctx.errors.push_back(fmt::format("unknow object type: {}", name));
            return {};
        }
        std::unordered_map<std::string, std::any> m;
        for (const auto& i : *e)
            m[i.first] = i.second;
        return ctx.space.create_object(name, mass, m);
    }>("make_object"),

    make<void, +[](eval_context& ctx, phy::object_builder o1, phy::object_builder o2, sf::Color c, double f, double d) -> std::any {
        ctx.space.create_special<phy::spring>(o1.get(), o2.get(), c, f, d);
        return {};
    }>("make_spring"),

    make<void, +[](eval_context& ctx, double sample_ticks, double sample_n, double width) -> std::any {
        return ctx.space.make_tracker(sample_ticks, (std::size_t) sample_n, width);
    }>("make_tracker"),

    make<void, +[](eval_context& ctx, double sample_ticks, double sample_n) -> std::any {
        return ctx.space.make_tracker(sample_ticks, (std::size_t) sample_n, 3);
    }>("make_tracker"),

    make<phy::tracker*, +[](eval_context& ctx, phy::object_builder obj, const std::string& n, sf::Color c) -> std::any {
        auto t = std::any_cast<phy::tracker*>(ctx.instance.value());
        if(!TYPES.contains(n))
            ctx.errors.push_back(fmt::format("unknown tracking type {}", n));
        else
            t->track(obj.get(), TYPES[n], c); 
        
        return t;
    }>("track"),

    make<void, +[](eval_context&, double x, double y) -> std::any {
        return phy::vec2d{x, y};
    }>("@__cons_vec"),

    make<void, +[](eval_context& ctx, double constant) -> std::any {
        ctx.builder->gravity(constant);
        return {};
    }>("@__cons_force_gravity"),

    make<void, +[](eval_context& ctx, double x, double y) -> std::any {
        ctx.builder->const_acc(x, y);
        return {};
    }>("@__cons_force_const_acc"),

    make<void, +[](eval_context& ctx, phy::vec2d v) -> std::any {
        ctx.builder->const_acc(v);
        return {};
    }>("@__cons_force_const_acc"),

    make<void, +[](eval_context& ctx, double d, double p) -> std::any {
        ctx.builder->force<phy::forces::force_drag>(d, (std::size_t)p);
        return {};
    }>("@__cons_force_drag"),
 
    make<void, +[](eval_context& ctx) -> std::any {
        ctx.builder->circle();
        return {};
    }>("@__cons_renderer_circle"),

    make<void, +[](eval_context& ctx, double scale) -> std::any {
        ctx.builder->render_acc(scale);
        return {};
    }>("@__cons_renderer_arrow_acc"),

    make<void, +[](eval_context& ctx, double scale) -> std::any {
        ctx.builder->render_vel(scale);
        return {};
    }>("@__cons_renderer_arrow_vel"),

    make<void, +[](eval_context& ctx, double min_dist) -> std::any {
        ctx.builder->trail(min_dist);
        return {};
    }>("@__cons_renderer_trail"),

    make<void, +[](eval_context& ctx, double min_dist) -> std::any {
        ctx.builder->trail(min_dist);
        return {};
    }>("@__cons_renderer_trail"),
        
    make<void, +[](eval_context& ctx, double cycles) -> std::any {
        ctx.space.set_cycles((std::size_t) cycles);
        return {};
    }>("engine_cycles_per"),
    
    make<void, +[](eval_context& ctx, double ticks) -> std::any {
        ctx.space.set_tick_mult(ticks);
        return {};
    }>("engine_ticks_mult"),
    
    make<phy::object_builder, +[](eval_context& ctx, double x, double y) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).pos(x, y);
        return ctx.instance.value();
    }>("pos"),

    make<phy::object_builder, +[](eval_context& ctx, double x, double y) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).vel(x, y);
        return ctx.instance.value();
    }>("vel"),

    make<phy::object_builder, +[](eval_context& ctx, double x, double y) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).momentum(x, y);
        return ctx.instance.value();
    }>("momentum"),

    make<phy::object_builder, +[](eval_context& ctx, phy::vec2d v) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).pos(v);
        return ctx.instance.value();
    }>("pos"),

    make<phy::object_builder, +[](eval_context& ctx, phy::vec2d v) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).vel(v);
        return ctx.instance.value();
    }>("vel"),

    make<phy::object_builder, +[](eval_context& ctx, phy::vec2d v) -> std::any {
        std::any_cast<phy::object_builder>(ctx.instance.value()).momentum(v);
        return ctx.instance.value();
    }>("momentum"),
};
// clang-format on

template <typename T>
class scope_guard
{
    T t;

public:
    scope_guard(T t) requires(std::is_nothrow_invocable_v<T>) : t(t) {}
    ~scope_guard() { t(); }
};

template <class T>
scope_guard(T) -> scope_guard<T>;

class call_expr_ast : public base_ast
{
    std::string callee;
    arg_list args;

public:
    virtual std::any eval(eval_context& ctx) const override
    {
        std::size_t this_hash = ctx.instance.has_value() ? ctx.instance.value().type().hash_code() : 0;
        std::vector<std::any> result;
        result.reserve(args.size());

        auto tmp = std::move(ctx.instance);
        ctx.instance = std::nullopt;
        for (const auto& i : args)
            result.push_back(safe_eval(ctx, i));
        ctx.instance = std::move(tmp);

        scope_guard g([&]() noexcept { ctx.instance = std::nullopt; });

        for (const auto& i : FN_HANDLES)
        {
            if (i.this_hash != this_hash)
                continue;

            if (callee != i.name)
                continue;
            if (i.type_hash.size() != args.size())
                continue;

            std::size_t index = 0;
            bool works = true;
            for (auto j : i.type_hash)
            {
                if (j != result[index++].type().hash_code())
                {
                    works = false;
                    break;
                }
            }

            if (!works)
                continue;

            return i.invoke(result, ctx);
        }

        ctx.errors.push_back(fmt::format("cannot find meaningful overload for \"{}\"", callee));
        return {};
    }

    virtual void dump_ast(std::ostream& os) const override
    {
        os << fmt::format("call expression ast (callee={})\n", callee);
        for (std::size_t i = 0; i < args.size(); i++)
        {
            os << fmt::format("arg {}:\n", i);
            args[i]->dump_ast(os);
        }
    }

    call_expr_ast(const std::string& callee, arg_list args)
        : base_ast(value_category::RVAL), callee(callee), args(std::move(args))
    {
    }
};

class dict_cons_expr_ast : public base_ast
{
    arg_map named_args;

public:
    virtual std::any eval(eval_context& ctx) const override
    {
        std::unordered_map<std::string, std::any> l;
        for (const auto& i : named_args)
            l[i.first] = safe_eval(ctx, i.second);

        return std::make_shared<decltype(l)>(l);
    }

    dict_cons_expr_ast(arg_map args) : base_ast(value_category::RVAL), named_args(std::move(args)) {}

    virtual void dump_ast(std::ostream& os) const override
    {
        os << "dict_cons_expression_ast\n";
        for (const auto& i : named_args)
        {
            os << fmt::format("arg \"{}\":\n", i.first);
            i.second->dump_ast(os);
        }
    }
};

using root_ast = std::vector<std::unique_ptr<base_ast>>;

class objtype_expr_ast : public base_ast
{
    arg_list forces;
    arg_list renderers;
    std::string name;
    std::string controller;

public:
    virtual std::any eval(eval_context& ctx) const override
    {
        if (controller == "default")
            ctx.builder = &ctx.space.create_class<phy::movement::default_controller>(name);
        else if (controller == "fixed")
            ctx.builder = &ctx.space.create_class<phy::movement::fixed_controller>(name);
        else
        {
            ctx.errors.push_back(fmt::format("unknown movement controller {}", controller));
            ctx.builder = &ctx.space.create_class<phy::movement::default_controller>(name);
        }

        for (const auto& i : forces)
            safe_eval(ctx, i);
        for (const auto& i : renderers)
            safe_eval(ctx, i);

        ctx.builder->build();
        return {};
    };

    objtype_expr_ast(arg_list forces, arg_list renderers, std::string name, std::string controller)
        : base_ast(value_category::RVAL), forces(std::move(forces)), renderers(std::move(renderers)), name(name),
          controller(controller)
    {
    }

    virtual void dump_ast(std::ostream& os) const override
    {
        os << fmt::format("objtype expression ast: name={} controller={}\n", name, controller);
        for (std::size_t i = 0; i < forces.size(); i++)
        {
            os << fmt::format("force {}:\n", i);
            forces[i]->dump_ast(os);
        }

        for (std::size_t i = 0; i < renderers.size(); i++)
        {
            os << fmt::format("renderers {}:\n", i);
            renderers[i]->dump_ast(os);
        }
    }
};

class member_access_ast : public base_ast
{
    std::unique_ptr<base_ast> lhs, rhs;

public:
    virtual std::any eval(eval_context& ctx) const override
    {
        auto hold = std::move(ctx.instance);
        ctx.instance = safe_eval(ctx, lhs);
        std::any res = safe_eval(ctx, rhs);
        ctx.instance = std::move(hold);
        return res;
    }

    virtual void dump_ast(std::ostream& os) const override
    {
        os << "member access expression ast\n";
        os << "lhs\n";
        lhs->dump_ast(os);
        os << "rhs\n";
        rhs->dump_ast(os);
    }

    member_access_ast(std::unique_ptr<base_ast> lhs, std::unique_ptr<base_ast> rhs)
        : base_ast(rhs->category()), lhs(std::move(lhs)), rhs(std::move(rhs))
    {
    }
};

#define expect(t, msg)                                                                                                      \
    do                                                                                                                      \
    {                                                                                                                       \
        if (tok.type() != t)                                                                                                \
        {                                                                                                                   \
            consume();                                                                                                      \
            return error(msg, tok.location());                                                                              \
        }                                                                                                                   \
    } while (0)

#define expect_ch(t, msg)                                                                                                   \
    do                                                                                                                      \
    {                                                                                                                       \
        if (tok != t)                                                                                                       \
        {                                                                                                                   \
            consume();                                                                                                      \
            return error(msg, tok.location());                                                                              \
        }                                                                                                                   \
    } while (0)

#define expect_fix(t, msg, fix)                                                                                             \
    do                                                                                                                      \
    {                                                                                                                       \
        if (tok.type() != t)                                                                                                \
        {                                                                                                                   \
            consume();                                                                                                      \
            return error(msg, tok.location(), fix);                                                                         \
        }                                                                                                                   \
    } while (0)

#define expect_ch_fix(t, msg, fix)                                                                                          \
    do                                                                                                                      \
    {                                                                                                                       \
        if (tok != t)                                                                                                       \
        {                                                                                                                   \
            consume();                                                                                                      \
            return error(msg, tok.location(), fix);                                                                         \
        }                                                                                                                   \
    } while (0)

class parser
{
    lexer lex;
    token tok;

    inline void consume() { tok = lex.next_token(); }

    inline std::unique_ptr<base_ast> error(const std::string& err, const src_location& loc, const std::string& fix = "")
    {
        lex.ctx().error(err, loc, fix);
        return nullptr;
    }

    std::unique_ptr<base_ast> parse_numeric_literal()
    {
        assert(tok.type() == token::TOK_LIT_NUMBER);
        double value = tok.number();
        consume();
        return std::make_unique<numeric_literal_ast>(value);
    }

    std::unique_ptr<base_ast> parse_string_literal()
    {
        assert(tok.type() == token::TOK_LIT_STR);
        std::string value = tok.str_lit();
        consume();
        return std::make_unique<string_literal_ast>(value);
    }

    std::unique_ptr<base_ast> parse_color_literal()
    {
        assert(tok.type() == token::TOK_LIT_COLOR);
        uint32_t value = tok.color();
        consume();
        return std::make_unique<color_literal_ast>(value);
    }

    std::optional<arg_list> parse_invoke_expr(char start = '(', char end = ')')
    {
        assert(tok == start);
        consume();

        arg_list params;

        if (tok != end)
        {
            while (true)
            {
                if (auto arg = parse_expr())
                    params.push_back(std::move(arg));
                else
                    return std::nullopt;

                if (tok == end)
                    break;

                if (tok != ',')
                {
                    lex.ctx().error("Expected ',' in argument list", tok.location());
                    return std::nullopt;
                }

                consume();
            }
        }
        consume();

        return params;
    }

    std::unique_ptr<base_ast> parse_vector_cons_expr()
    {
        auto args = parse_invoke_expr('[', ']');
        if (!args)
            return nullptr;
        return std::make_unique<call_expr_ast>("@__cons_vec", std::move(args.value()));
    }

    std::unique_ptr<base_ast> parse_dict_cons_expr()
    {
        assert(tok == '{');
        consume();

        arg_map params;

        if (tok != '}')
        {
            while (true)
            {
                std::string name;
                expect_fix(token::TOK_IDENTIFIER, "Expected identifier in dictionary construction", "Remove trailing comma");
                name = tok.identifier();

                consume();
                expect_ch(':', "Expected ':' after identifier in dictionary construction");
                consume();
                if (auto arg = parse_expr())
                    params.insert({name, std::move(arg)});
                else
                    return nullptr;

                if (tok == '}')
                    break;

                expect_ch(',', "Expected ',' in argument list");
                consume();
            }
        }
        consume();

        return std::make_unique<dict_cons_expr_ast>(std::move(params));
    }

    std::unique_ptr<base_ast> parse_objtype_expr()
    {
        assert(tok.type() == token::TOK_KW_OBJTYPE);
        consume();
        expect(token::TOK_IDENTIFIER, "expected identifier");

        std::string name = tok.identifier();
        std::string control = "default";
        consume();
        if (tok.type() == token::TOK_KW_CONTROL)
        {
            consume();
            expect(token::TOK_IDENTIFIER, "expected identifier");
            control = tok.identifier();
            consume();
        }

        expect_ch('{', "expected open bracket");
        consume();

        arg_list forces;
        arg_list renderers;
        while (tok != '}')
        {
            if (tok.type() == token::TOK_KW_FORCE)
            {
                consume();
                expect(token::TOK_IDENTIFIER, "expected identifier");
                std::string name = tok.identifier();
                consume();
                auto args = parse_invoke_expr();
                if (!args)
                    return nullptr;

                forces.push_back(std::make_unique<call_expr_ast>("@__cons_force_" + name, std::move(args.value())));

                expect(token::TOK_SEPERATOR, "expected semicolon");
                consume();
            }
            else if (tok.type() == token::TOK_KW_RENDERER)
            {
                consume();
                expect(token::TOK_IDENTIFIER, "expected identifier");
                std::string name = tok.identifier();
                consume();
                auto args = parse_invoke_expr();
                if (!args)
                    return nullptr;

                renderers.push_back(std::make_unique<call_expr_ast>("@__cons_renderer_" + name, std::move(args.value())));

                expect(token::TOK_SEPERATOR, "expected semicolon");
                consume();
            }
            else
                return error("unexpected token; expected 'force' or 'renderer' keyword", tok.location());
        }

        consume();
        return std::make_unique<objtype_expr_ast>(std::move(forces), std::move(renderers), name, control);
    }

    std::unique_ptr<base_ast> parse_paren_expr()
    {
        assert(tok == '(');
        consume();
        auto expr = parse_expr();
        if (!expr)
            return nullptr;

        expect_ch('(', "expected ')'");
        consume();
        return expr;
    }

    int tok_precedence()
    {
        constexpr static std::pair<char, int> BINOP_PRECEDENCE[] = {{'=', 100}, {'+', 200}, {'-', 200},
                                                                    {'*', 400}, {'/', 400}, {'%', 400}};

        if (tok.type() != token::TOK_OPERATOR)
            return -1;

        for (const auto& i : BINOP_PRECEDENCE)
            if (i.first == tok.ch())
                return i.second;
        return -1;
    }

    std::unique_ptr<base_ast> parse_binop_rhs(int expr_prec, std::unique_ptr<base_ast> lhs)
    {
        while (true)
        {
            int tok_prec = tok_precedence();

            if (tok_prec < expr_prec)
                return lhs;

            auto bin_op = tok.ch();
            consume();
            auto rhs = parse_primary_expr();
            if (!rhs)
                return nullptr;

            int next_prec = tok_precedence();
            if (tok_prec < next_prec)
            {
                rhs = parse_binop_rhs(tok_prec + 1, std::move(rhs));
                if (!rhs)
                    return nullptr;
            }

            lhs = std::make_unique<binary_expr_ast>(bin_op, std::move(lhs), std::move(rhs));
        }
    }

    std::unique_ptr<base_ast> parse_identifier()
    {
        assert(tok.type() == token::TOK_IDENTIFIER);
        std::string name = tok.identifier();
        consume();

        if (tok == '(')
        {
            auto e = parse_invoke_expr();
            if (!e)
                return nullptr;
            return std::make_unique<call_expr_ast>(name, std::move(e.value()));
        }

        return std::make_unique<variable_expr_ast>(name);
    }

    std::unique_ptr<base_ast> parse_simple_expr()
    {
        switch (tok.type())
        {
        case token::TOK_LIT_NUMBER:
            return parse_numeric_literal();
        case token::TOK_LIT_STR:
            return parse_string_literal();
        case token::TOK_LIT_COLOR:
            return parse_color_literal();
        case token::TOK_CH:
            if (tok.ch() == '[')
                return parse_vector_cons_expr();
            else if (tok.ch() == '{')
                return parse_dict_cons_expr();
            else if (tok.ch() == '(')
                return parse_paren_expr();
            else
            {
                lex.ctx().error("invalid character", tok.location());
                return nullptr;
            }
        case token::TOK_IDENTIFIER:
            return parse_identifier();
        default:
            lex.ctx().error("invalid token", tok.location());
            return nullptr;
        }
    }

    std::unique_ptr<base_ast> parse_primary_expr()
    {
        auto lhs = parse_simple_expr();
        if (!lhs)
            return nullptr;

        while (tok == '.')
        {
            consume();
            expect(token::TOK_IDENTIFIER, "expected identifier after member access");

            auto expr = parse_identifier();
            if (!expr)
                return nullptr;
            lhs = std::make_unique<member_access_ast>(std::move(lhs), std::move(expr));
        }
        return lhs;
    }

    std::unique_ptr<base_ast> parse_expr()
    {
        auto lhs = parse_primary_expr();
        if (!lhs)
            return nullptr;
        return parse_binop_rhs(0, std::move(lhs));
    }

    std::unique_ptr<base_ast> parse_statement()
    {
        if (tok.type() == token::TOK_KW_OBJTYPE)
            return parse_objtype_expr();
        else
        {
            std::unique_ptr<base_ast> ret;
            ret = parse_expr();
            expect(token::TOK_SEPERATOR, "expected ';' at end of statement");
            consume();
            return ret;
        }
    }

public:
    root_ast parse()
    {
        root_ast ast;
        while (tok.type() != token::TOK_EOF)
            ast.push_back(parse_statement());

        return ast;
    }

    parser(parse_context&& ctx) : lex(std::forward<parse_context>(ctx)), tok({0, 0}, ' ') { consume(); }
    void dump_errors(std::ostream& os) { lex.dump_errors(os); }
    bool had_errors() const { return lex.had_errors(); }
};

#include <fstream>
#include <iostream>

root_ast parse_file(const std::string& file)
{
    std::ifstream ifs(file);
    parser p(parse_context(ifs, file.c_str()));
    auto v = p.parse();
    p.dump_errors(std::cout);
    if (p.had_errors())
        exit(-1);
    return v;
}

#include <ranges>

phy::physics_space create_space(const std::string& file, sf::RenderWindow& rw, sf::Font& f, double subtick_mult,
                                std::size_t cycles)
{
    auto ast = parse_file(file);
    phy::physics_space space(rw, f, subtick_mult, cycles);

    std::unordered_map<std::string, std::any> m;

    eval_context ctx{m, {}, {}, nullptr, space};

    for (const auto& i : ast)
        safe_eval(ctx, i);

    logging::logger_ref ref("phyconf-parse");

    for (const auto& e : ctx.errors)
        ref.error(e);

    if (ctx.errors.size() != 0)
        exit(-1);

    return space;
}
