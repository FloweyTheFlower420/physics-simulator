#ifndef __PHY_COMPONENT_RENDERERS_TRAIL_RENDERER_H__
#define __PHY_COMPONENT_RENDERERS_TRAIL_RENDERER_H__
#include <SFML/Graphics.hpp>
#include <component/renderer.h>

namespace phy::render
{
    class trail_renderer : public renderer
    {
        indexed_type<sf::VertexArray> vert;
        indexed_type<sf::Color> trail_color;
        double min_dist;

    public:
        inline static constexpr named_type<sf::Color> COLOR_KEY = "r@trail::color";

        trail_renderer(const slot_allocator& alloc, double min_dist);

        virtual void init(object& that, const named_value_map& map) override;
        virtual void update_phase(object& that) override;
        virtual void render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state) override;
        virtual ~trail_renderer() override = default;
    };

} // namespace phy::render

#endif
