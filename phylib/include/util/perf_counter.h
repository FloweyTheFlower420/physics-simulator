#ifndef __PHY_PERF_COUNTER_H__
#define __PHY_PERF_COUNTER_H__
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <ratio>
#include <string>
#include <util/chrono_util.h>

namespace phy
{
    class framerate_counter
    {
        std::size_t rendered_frames = 0;
        std::size_t max_frames = 0;
        std::string msg = "unk/unk";
        tick_counter<std::chrono::microseconds> counter;
        double t = 0;

    public:
        void update();
        constexpr const std::string& get() const { return msg; }
    };
} // namespace phy

#endif
