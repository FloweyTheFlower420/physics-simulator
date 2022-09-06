#include <object.h>

namespace phy
{
    object::object(double mass, object_class* clazz, const named_value_map& v) : clazz(clazz), mass(mass > 0 ? mass : 1)
    {
        clazz->init_object(*this, v);
    }

    vec2d object::apply_force(object& obj)
    {
        vec2d v;
        for (auto& i : this->clazz->forces)
            v += i->compute_force(*this, obj);
        return v;
    }

    void object::update(double dt, const vec2d& force) { this->clazz->controller->update(*this, dt, force); }

    void object::step_time()
    {
        acc = new_acc;
        vel = new_vel;
        pos = new_pos;

        for (auto& i : this->clazz->renderers)
            i->update_phase(*this);
    }

    void object::draw(sf::RenderTarget& t, sf::RenderStates s) const
    {
        for (const auto& i : this->clazz->renderers)
            i->render_phase(*this, t, s);
    }

    object::~object() { clazz->destroy_object(*this); }
} // namespace phy
