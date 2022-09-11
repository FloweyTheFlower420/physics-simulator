#ifndef __PHY_COMPONENT_RENDERERS_CIRCLE_RENDERER_H__
#define __PHY_COMPONENT_RENDERERS_CIRCLE_RENDERER_H__
#include <SFML/Graphics.hpp>
#include <component/renderer.h>

namespace phy::render
{
    class circle_renderer : public renderer
    {
        indexed_type<sf::CircleShape> circle;

    public:
        inline static constexpr named_type<sf::Color> COLOR_KEY = "render_circle_color";
        inline static constexpr named_type<double> RADIUS_KEY = "render_circle_radius";
        circle_renderer(const slot_allocator& alloc);

        virtual void init(object& that, const named_value_map& map) override;
        virtual void update_phase(object& that) override;
        virtual void render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state) override;
        virtual ~circle_renderer() override = default;
    };
} // namespace phy::render

#endif
