#include <component/renderers/circle_renderer.h>
#include <fmt/format.h>
#include <object.h>

namespace phy::render
{
    circle_renderer::circle_renderer(const slot_allocator& alloc) : circle(alloc_slot<sf::CircleShape>(alloc)) {}

    void circle_renderer::update_phase(object& that)
    {
        sf::CircleShape& shape = *circle.get(that.get_valuemap());
        shape.setPosition(that.get_pos()[0] - shape.getRadius(), that.get_pos()[1] - shape.getRadius());
    }

    void circle_renderer::render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state)
    {
        sf::CircleShape& shape = *circle.get(that.get_valuemap());
        tgt.draw(shape, state);
    }

    void circle_renderer::init(object& that, const named_value_map& map)
    {
        const sf::Color& color = COLOR_KEY.at(map);
        double radius = RADIUS_KEY.at(map);

        sf::CircleShape* shape = new sf::CircleShape(radius);
        shape->setFillColor(color);

        circle.write(that.get_valuemap(), shape);
    }
} // namespace phy::render
