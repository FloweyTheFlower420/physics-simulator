#include <SFML/Graphics/PrimitiveType.hpp>
#include <physics.h>
#include <special_object.h>

namespace phy
{
    spring::spring(const object& o1, const object& o2, sf::Color color, double spring_const, double relaxed_len)
        : o1(o1), o2(o2), color(color), spring_const(spring_const), relaxed_len(relaxed_len)
    {
    }

    void spring::handle_forces(physics_space& space, std::vector<vec2d>& vec, double dt)
    {
        vec2d disp = o1.get_pos() - o2.get_pos();
        double delta = disp.magnitude() - relaxed_len;
        double force = delta * spring_const;

        vec[o1.identifier()] += -disp.normalize() * force;
        vec[o2.identifier()] += disp.normalize() * force;
    }

    void spring::handle_update(physics_space& space, double dt)
    {
        // nop
    }

    void spring::handle_step_time()
    {
        // nop
    }

    void spring::handle_render(sf::RenderTarget& target)
    {
        sf::Vertex verts[]{{vector_cast<float>(o1.get_pos()), color}, {vector_cast<float>(o2.get_pos()), color}};
        target.draw(verts, 2, sf::LineStrip);
    }
} // namespace phy
