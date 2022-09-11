#ifndef __PHYLIB_SPECIAL_OBJECT_H__
#define __PHYLIB_SPECIAL_OBJECT_H__

#include <SFML/Graphics/RenderTarget.hpp>
#include <boost/circular_buffer.hpp>
#include <object.h>
#include <util/vec.h>
#include <vector>
namespace phy
{
    class physics_space;

    class special_object
    {
    public:
        virtual void handle_forces(physics_space& space, std::vector<vec2d>& vec, double dt) = 0;
        virtual void handle_update(physics_space& space, double dt) = 0;
        virtual void handle_step_time() = 0;
        virtual void handle_render(sf::RenderTarget&) = 0;
        virtual ~special_object() = default;
    };

    class spring : public special_object
    {
        const object& o1;
        const object& o2;
        double spring_const;
        double relaxed_len;
        sf::Color color;

    public:
        spring(const object& o1, const object& o2, sf::Color color, double spring_const, double relaxed_len);
        virtual void handle_forces(physics_space& space, std::vector<vec2d>& vec, double dt) override;
        virtual void handle_update(physics_space& space, double dt) override;
        virtual void handle_step_time() override;
        virtual void handle_render(sf::RenderTarget&) override;
        virtual ~spring() = default;
    };
} // namespace phy

#endif
