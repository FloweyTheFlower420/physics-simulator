#ifndef __PHY_COMPONENT_MOVEMENT_H__
#define __PHY_COMPONENT_MOVEMENT_H__
#include <util/vec.h>

namespace phy
{
    class object;
    namespace movement
    {
        class movement_controller
        {
        public:
            virtual void update(object& obj, float dt, const vec2d& vec) = 0;
            virtual ~movement_controller() = default;
        };

        class default_controller : public movement_controller
        {
        public:
            virtual void update(object& obj, float dt, const vec2d& vec);
            virtual ~default_controller() = default;
        };

        class fixed_controller : public movement_controller
        {
        public:
            virtual void update(object& obj, float dt, const vec2d& vec);
            virtual ~fixed_controller() = default;
        };
    } // namespace movement
} // namespace phy

#endif
