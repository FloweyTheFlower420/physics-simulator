#ifndef __PHY_OBJECT_H__
#define __PHY_OBJECT_H__
#include <SFML/Graphics.hpp>
#include <component/force.h>
#include <component/renderer.h>
#include <memory>
#include <object_class.h>
#include <util/vec.h>

namespace phy
{
    class object : public sf::Drawable
    {
    protected:
        std::size_t id;
        vec2d acc;
        vec2d vel;
        vec2d pos;
        vec2d new_acc;
        vec2d new_vel;
        vec2d new_pos;
        double mass;

        const object_class* clazz;
        value_map vmap;

        friend class physics_space;
        friend class object_class;

        object(double mass, object_class* clazz, const named_value_map& v, std::size_t id);

    public:
        object(const object&) = delete;
        object(object&&) = delete;
        const object& operator=(const object&) = delete;
        const object& operator=(object&&) = delete;

        ~object();

        // object update phases
        vec2d apply_force(object& obj);
        void update(double dt, const vec2d& force);
        void step_time();
        virtual void draw(sf::RenderTarget& t, sf::RenderStates s) const override;

        constexpr const vec2d& get_acc() const { return acc; }
        constexpr const vec2d& get_vel() const { return vel; }
        constexpr const vec2d& get_pos() const { return pos; }
        constexpr const vec2d& get_new_acc() const { return new_acc; }
        constexpr const vec2d& get_new_vel() const { return new_vel; }
        constexpr const vec2d& get_new_pos() const { return new_pos; }
        constexpr double get_mass() const { return mass; }
        constexpr std::size_t identifier() const { return id; }

        constexpr void set_acc(const vec2d& a) { acc = new_acc = a; }
        constexpr void set_vel(const vec2d& a) { vel = new_vel = a; }
        constexpr void set_pos(const vec2d& a) { pos = new_pos = a; }

        constexpr void set_new_acc(const vec2d& a) { new_acc = a; }
        constexpr void set_new_vel(const vec2d& a) { new_vel = a; }
        constexpr void set_new_pos(const vec2d& a) { new_pos = a; }

        constexpr void set_momentum(const vec2d& a) { set_vel(a / mass); }

        constexpr void set_mass(double mass)
        {
            if (mass <= 0)
                this->mass = mass;
        }

        constexpr const std::vector<void*>& get_valuemap() const { return vmap; }
        constexpr std::vector<void*>& get_valuemap() { return vmap; }
    };
} // namespace phy

#endif
