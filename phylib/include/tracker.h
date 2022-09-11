#ifndef __PHY_TRACKER_H__
#define __PHY_TRACKER_H__
#include <boost/circular_buffer.hpp>
#include <object.h>

namespace phy
{
    enum class statspec_types
    {
        POS,
        VEL,
        MOMENTUM,
        ACC,
        FORCE, // track magnitude
        POS_X,
        VEL_X,
        MOMENTUM_X,
        ACC_X,
        FORCE_X,
        POS_Y,
        VEL_Y,
        MOMENTUM_Y,
        ACC_Y,
        FORCE_Y,
        KE,
    };

    struct tracked_object
    {
        object& obj;
        statspec_types type;
        sf::Color c;
    };

    class physics_space;
    class tracker
    {
        std::vector<tracked_object> objects;
        std::vector<boost::circular_buffer<double>> buf;
        std::size_t sample_n;
        double ticks;
        const double sample_ticks;
        double width;

    public:
        tracker(double sample_ticks, std::size_t sample_n, double width);
        void handle_update(physics_space& space, double dt);
        void handle_render(sf::RenderTarget&);

        inline void track(object& obj, statspec_types t, sf::Color c)
        {
            buf.push_back(boost::circular_buffer<double>(sample_n));
            objects.push_back({obj, t, c});
        }

        ~tracker() = default;
    };
} // namespace phy

#endif
