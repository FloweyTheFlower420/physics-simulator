#include <cstddef>

namespace phy
{
    struct conststr
    {
        const char* const p;
        template <std::size_t N>
        constexpr conststr(const char (&a)[N]) : p(a) /*, sz(N - 1) */
        {
        }
    };
} // namespace phy
