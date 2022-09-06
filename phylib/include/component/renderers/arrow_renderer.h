#ifndef __PHY_COMPONENT_RENDERERS_ARROW_RENDERER_H__
#define __PHY_COMPONENT_RENDERERS_ARROW_RENDERER_H__
#include <SFML/Graphics.hpp>
#include <component/renderer.h>
#include <util/const_str.h>
#include <object.h>

namespace phy::render
{
    enum  arrow_type
    {
        VEL,
        ACC
    };

    template <arrow_type T>
    class arrow_renderer : public renderer
    {
        double scale;
        indexed_type<sf::ConvexShape> triangle;
        indexed_type<sf::Vertex> vert;

        constexpr static const char* get_key_name()
        {
            switch (T)
            {
            case VEL:
                return "r@arrow_vel::color";
            case ACC:
                return "r@arrow_acc::color";
            }
        }

    public:
        inline static constexpr named_type<sf::Color> COLOR_KEY = get_key_name();

        arrow_renderer(const slot_allocator& alloc, double scale)
            : scale(scale), triangle(alloc_slot<sf::ConvexShape>(alloc)), vert(alloc_slot<sf::Vertex, true>(alloc))
        {
        }

        virtual void init(object& that, const named_value_map& map) override
        {
            const sf::Color& color = COLOR_KEY.at(map);

            sf::ConvexShape* trig = new sf::ConvexShape(3);
            trig->setPoint(0, {2, 0});
            trig->setPoint(1, {-2, 0});
            trig->setPoint(2, {0, 4});
            trig->setFillColor(color);

            auto vert = new sf::Vertex[2]();
            vert[0].color = color;
            vert[1].color = color;

            triangle.write(that.get_valuemap(), trig);
            this->vert.write(that.get_valuemap(), vert);
        }

        virtual void update_phase(object& that) override
        {
            const vec2d& vec = T == VEL ? that.get_vel() : that.get_acc();
            vec2d result = that.get_pos() + vec * scale;
            auto& triangle = *this->triangle.get(that.get_valuemap());
            sf::Vertex* v = this->vert.get(that.get_valuemap());

            triangle.setRotation(360 - vec.angle());
            triangle.setPosition(vector_cast<float>(result));
            v[0].position = vector_cast<float>(that.get_pos());
            v[1].position = vector_cast<float>(result);
        }

        virtual void render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state) override
        {
            tgt.draw(*triangle.get(that.get_valuemap()), state);
            tgt.draw(vert.get(that.get_valuemap()), 2, sf::Lines);
        }
    };

} // namespace phy::render
#endif
