#ifndef __PHY_INCLUDE_UTIL_CHRONO_UTIL_H__
#define __PHY_INCLUDE_UTIL_CHRONO_UTIL_H__
#include <chrono>

namespace phy
{
    template <typename T>
    class tick_counter
    {
        T n;
        std::chrono::high_resolution_clock clk;

    public:
        tick_counter() : clk() { n = std::chrono::duration_cast<T>(clk.now().time_since_epoch()); }
        double dt()
        {
            auto curr_time = std::chrono::duration_cast<T>(clk.now().time_since_epoch());
            auto dt = (curr_time - n).count() * ((double)T::period::num / T::period::den);
            n = curr_time;
            return dt;
        }
    };
} // namespace phy

#endif
