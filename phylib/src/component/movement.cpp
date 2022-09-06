#include <component/movement.h>
#include <fmt/ranges.h>
#include <logging.h>
#include <object.h>

namespace phy::movement
{
    void default_controller::update(object& obj, float dt, const vec2d& vec)
    {
        obj.set_new_acc(vec / obj.get_mass());
        obj.set_new_pos(obj.get_pos() + obj.get_vel() * dt);
        auto dv = obj.get_acc() * dt;
        obj.set_new_vel(obj.get_vel() + dv);

        if(dv.magnitude() > 750) 
        {
            logging::logger::get_instance().nwarn("force::gravity", fmt::format("huge delta velocity of {}", dv.magnitude()));
        }

        if (obj.get_vel().magnitude() > 1000000)
        {
            logging::logger::get_instance().nwarn("movement::default",
                                                  fmt::format("velocity limit exceeded {}", obj.get_vel()));
            obj.set_new_vel(obj.get_vel().normalize() * 1000000);
        }
    }
} // namespace phy::movement
