#ifndef __PHY_FORCES_H__
#define __PHY_FORCES_H__
#include <util/vec.h>

namespace phy
{
    class object;

    namespace forces
    {
        class force
        {
        public:
            virtual vec2d compute_force(object& that, object& rhs) = 0;
            virtual ~force() = default;
        };

        class gravity final : public force
        {
            const double constant;

        public:
            constexpr gravity(double G) : constant(G) {}

            virtual vec2d compute_force(object& that, object& rhs) override;
        };

        class simple_field final : public force
        {
            const double constant;
            const double power;

        public:
            constexpr simple_field(double G, double power) : constant(G), power(power) {}

            virtual vec2d compute_force(object& that, object& rhs) override;
        };

        class force_drag final : public force
        {
            const double drag_const;
            const double power;

        public:
            constexpr force_drag(double drag_const, double power) : drag_const(drag_const), power(power) {}

            virtual vec2d compute_force(object& that, object& rhs) override;
        };

        class const_acc final : public force
        {
            vec2d acc;

        public:
            const_acc(vec2d acc) : acc(acc) {}

            virtual vec2d compute_force(object& that, object& rhs) override;
        };
    } // namespace forces
} // namespace phy

#endif
