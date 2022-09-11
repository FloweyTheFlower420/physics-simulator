# Keyword
A *keyword* is an *identifier* that is reserved by the language.
    - `objtype` starts an *objtype-decl*
    - `control` specifies an object type controller
    - `force` specifies a force
    - `renderer` specifies a renderer

# Identifiers
A *identifier* is a token that matches `[a-zA-Z][a-zA-Z0-9]*` that is not a keyword

# Literals:
*literal* 
  ::= *string-literal*
  ::= *numeric-literal*
  ::= *color-literal*

## String literals
A *string-literal* must start and end with `"`
The characters allowed in a string literal shall match `[!-~ \t]`

If a character not within the allowed character set, escape sequences must be used. 
An escape sequence must start with the `\` character, followed by one of the following:

| Character    | Behavior                |
| ------------ | ----------------------- |
| a            | Same as C               |
| b            | Same as C               |
| e            | Equal to \x1b           |
| f            | Same as C               |
| n            | Same as C               |
| r            | Same as C               |
| \            | Same as C               |
| '            | Same as C               |
| "            | Same as C               |
| x[\da-f]{2}  | Emits char based on hex |
| [0-7]{1,3}   | Emits char based on oct |

## Numeric literals
A valid floating point value that can be parsed by `std::strtod`

## Color literals
A color literal must match `#[\da-f]{6}`, and will be interpreted as a RGB color

# Operator Expression
An operator expression consists of values and operators.
*operator-expr* ::= *expression* *operator* *expression*

# Invoke Expression
An argument is a list of arguments for a *function-call* or a *member-function-call*
*invoke-expr* ::= '(' *expression*, ... ')'

# Function Call
*function-call* ::= *identifier* *invoke-expr*

# Vector Cons
A vector construction expression.
[ *expression*, ... ]

# Dict Cons
*dict-cons* ::= '{' *identifier*: *expression*, ... '}'

# Paren Expression
*paren-expr* ::= '(' *identifier* ')'

# Expression
*expresion* 
  ::= *paren-expr*
  ::= *literal*
  ::= *operator-expr*
  ::= *function-call*
  ::= *member-function-call*
  ::= *vector-cons*
  ::= *dict-cons*

# Objtype Decl
Declares an object type.  
*objtype-decl* 
  ::= *kw-objtype* *identifier* [ *kw-control* *identifier* ] '{' *objtype-decl-body* '}'

The body of this declaration consists of:
    - *kw-force* *identifier* *invoke-expr*; 
    - *kw-renderer* *identifier* *invoke-expr*;

# Statement
*statement* 
  ::=*expression* ';'
  ::= *objtype-decl*

# Language functions:
    - `make_object(string clazz, number mass, dictionary param_map) -> object`
    - `make_spring(object object_1, object object_2, color c, number spring_const, number default_len) -> void`
    - `engine_cycles_per(number cycles) -> void`
    - `engine_ticks_mult(number multiplier) -> void`
    - `object::pos(number x, number y) -> object`
    - `object::vel(number x, number y) -> object`
    - `object::momentum(number x, number y) -> object`
    - `object::pos(vec2 v) -> object`
    - `object::vel(vec2 v) -> object`
    - `object::momentum(vec2 v) -> object`
    - `force gravity(number constant)`
    - `force const_acc(vec2 force)`
    - `force const_acc(number x, number y)`
    - `force drag(number constant, number exp)`
    - `renderer circle()`
    - `renderer arrow_acc/arrow_vel(number scale)`
    - `renderer trail(number min_dist_before_update)`
Valid controllers:
    - `default`
    - `fixed`
