#include <SFML/Graphics.hpp>
#include <component/renderers/trail_renderer.h>
#include <object.h>

namespace phy::render
{

    trail_renderer::trail_renderer(const slot_allocator& alloc, double min_dist)
        : vert(alloc_slot<sf::VertexArray>(alloc)), min_dist(min_dist),
          trail_color(alloc_slot<sf::Color>(alloc))
    {
    }

    void trail_renderer::init(object& that, const named_value_map& map)
    {
        const sf::Color& color = COLOR_KEY.at(map);

        sf::VertexArray* varray = new sf::VertexArray(sf::PrimitiveType::LineStrip);

        vert.write(that.get_valuemap(), varray);
        trail_color.write(that.get_valuemap(), new sf::Color(color));
    }

    void trail_renderer::update_phase(object& that)
    {
        sf::VertexArray& vert = *this->vert.get(that.get_valuemap());
        const sf::Color& trail_color = *this->trail_color.get(that.get_valuemap());

        sf::Vector2f vert_pos = vector_cast<float>(that.get_pos());
        if (vert.getVertexCount() != 0)
        {
            vec2d old1 = vector_cast<double>(vert[vert.getVertexCount() - 2].position);
            vec2d old2 = vector_cast<double>(vert[vert.getVertexCount() - 1].position);
            double magnitude = (old1 - old2).magnitude();
            if (magnitude <= min_dist)
                vert.resize(vert.getVertexCount() - 1);
            vert.append({vert_pos, trail_color});
        }
        else
        {
            vert.append({vert_pos, trail_color});
            vert.append({vert_pos, trail_color});
        }
    }

    void trail_renderer::render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state)
    {
        const sf::VertexArray& vert = *this->vert.get(that.get_valuemap());
        tgt.draw(vert, state);
    }
} // namespace phy::render
