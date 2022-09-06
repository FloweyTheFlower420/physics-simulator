#ifndef __PHY_RENDERER_H__
#define __PHY_RENDERER_H__
#include <SFML/Graphics.hpp>
#include <util/obj_class_util.h>
#include <util/vec.h>

namespace phy
{
    class object;

    namespace render
    {
        class renderer
        {
        public:
            virtual void init(object& that, const named_value_map& map) = 0;
            virtual void update_phase(object& that) = 0;
            virtual void render_phase(const object& that, sf::RenderTarget& tgt, sf::RenderStates state) = 0;
            virtual ~renderer() = default;
        };
    } // namespace render
} // namespace phy

#endif
