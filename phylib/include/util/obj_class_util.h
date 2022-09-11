#ifndef __PHY_UTIL_OBJ_CLASS_UTIL_H__
#define __PHY_UTIL_OBJ_CLASS_UTIL_H__
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

namespace phy
{
    using value_map = std::vector<void*>;
    using named_value_map = std::unordered_map<std::string, std::any>;

    template <typename T>
    class indexed_type
    {
        std::size_t v;

    public:
        constexpr indexed_type(std::size_t index) : v(index) {}
        constexpr indexed_type(const indexed_type&) = default;
        constexpr indexed_type(indexed_type&&) = default;
        constexpr T* get(const value_map& vmap) const { return (T*)vmap[v]; }
        constexpr void write(value_map& vmap, T* new_val) const { vmap[v] = new_val; }
    };

    template <typename T>
    class named_type
    {
        const char* v;

    public:
        constexpr named_type(const char* index) : v(index) {}
        constexpr named_type(const named_type&) = default;
        constexpr named_type(named_type&&) = default;
        constexpr T* get(const named_value_map& vmap) const { return (T*)vmap.at(v); }

        template <typename... Args>
        std::pair<std::string, std::any> operator()(Args&&... args) const
        {
            return std::make_pair<std::string, std::any>(v, T(std::forward<Args>(args)...));
        }

        constexpr const T& at(const named_value_map& map) const { return std::any_cast<const T&>(map.at(v)); }
    };

    template <typename T, typename R, typename... Args>
    class bound_pmf
    {
        T* that;
        R (T::*fn)(Args...);

    public:
        constexpr bound_pmf(T* inst, R (T::*fn)(Args...)) : that(inst), fn(fn) {}
        template <typename... _Args>
        R invoke(_Args&&... args) const
        {
            return (that->*fn)(std::forward<Args>(args)...);
        }
    };

    class object_class_builder;
    using slot_allocator = bound_pmf<object_class_builder, std::size_t, const char*, void (*)(void*)>;

    template <typename T, bool B = false>
    indexed_type<T> alloc_slot(const slot_allocator& alloc, const char* str = nullptr)
    {
        const char* str2 = str;
        return indexed_type<T>(alloc.invoke(
            str2, +[](void* p) {
                if constexpr (B)
                    delete[]((T*)p);
                else
                    delete ((T*)p);
            }));
    }

    using index_map = std::unordered_map<std::string, std::size_t>;
} // namespace phy

#endif
