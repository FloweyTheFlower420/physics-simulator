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
            const std::size_t power;
        public:
            constexpr simple_field(double G, std::size_t power) : constant(G), power(power) {}

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
