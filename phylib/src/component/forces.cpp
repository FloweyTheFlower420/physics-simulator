#include <component/force.h>
#include <fmt/ranges.h>
#include <logging.h>
#include <object.h>

namespace phy::forces
{
    vec2d gravity::compute_force(object& that, object& rhs)
    {
        if (&that != &rhs)
        {
            vec2d dist = rhs.get_pos() - that.get_pos();

            if (dist.magnitude() < 0.1)
            {
                logging::logger::get_instance().nwarn("force::gravity", fmt::format("small displacement of {}", dist));
                dist.magnitude(0.1);
            }

            double r_sq = dist.dot(dist);

            return -dist.normalize() * constant * that.get_mass() * rhs.get_mass() / r_sq;
        }

        return vec2d();
    }

    vec2d const_acc::compute_force(object& that, object& rhs)
    {
        if (&that == &rhs)
            return acc * that.get_mass();
        return vec2d();
    }

    vec2d force_drag::compute_force(object& that, object& rhs)
    {
        if (&that == &rhs)
            return -that.get_vel().normalize() * std::pow(that.get_vel().magnitude(), power) * drag_const;
        return vec2d();
    }

    vec2d simple_field::compute_force(object& that, object& rhs)
    {
        if (&that != &rhs)
        {
            vec2d dist = rhs.get_pos() - that.get_pos();
            double r_sqrt = invsqrt(dist.dot(dist));
            return -dist.normalize() * constant * that.get_mass() * rhs.get_mass() * std::pow(r_sqrt, power);
        }

        return vec2d();
    }
} // namespace phy::forces
