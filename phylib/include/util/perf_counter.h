#ifndef __PHY_PERF_COUNTER_H__
#define __PHY_PERF_COUNTER_H__
#include "util/chrono_util.h"
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <ratio>
#include <string>

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
        constexpr void update()
        {
            double dt = counter.dt();
            t += dt;
            if (1 / dt > max_frames)
                max_frames = 1 / dt;
            if (t > 1)
            {
                msg = fmt::format("{}/{}", rendered_frames, max_frames);
                t = 0;
                rendered_frames = 0;
                max_frames = 0;
            }

            rendered_frames++;
        }

        constexpr const std::string& get() const { return msg; }
    };
} // namespace phy

#endif
