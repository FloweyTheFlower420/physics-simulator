#include <util/perf_counter.h>

void phy::framerate_counter::update()
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

