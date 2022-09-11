#ifndef __PHY_BUILDER_H__
#define __PHY_BUILDER_H__
#include <component/force.h>
#include <component/movement.h>
#include <component/renderer.h>
#include <component/renderers/arrow_renderer.h>
#include <component/renderers/circle_renderer.h>
#include <component/renderers/trail_renderer.h>
#include <memory>
#include <object.h>
#include <string>
#include <unordered_map>
#include <util/obj_class_util.h>

namespace phy
{
    class physics_space;

    class object_builder
    {
        object& obj;
        friend class physics_space;
        constexpr object_builder(object& obj) : obj(obj) {}

    public:
        constexpr object_builder& vel(const vec2d& ve)
        {
            obj.set_vel(ve);
            return *this;
        }

        constexpr object_builder& pos(const vec2d& ve)
        {
            obj.set_pos(ve);
            return *this;
        }

        constexpr object_builder& momentum(const vec2d& ve)
        {
            obj.set_momentum(ve);
            return *this;
        }

        constexpr object_builder& vel(double x, double y) { return vel({x, y}); }
        constexpr object_builder& pos(double x, double y) { return pos({x, y}); }
        constexpr object_builder& momentum(double x, double y) { return momentum({x, y}); }

        constexpr object& get() const { return obj; }
    };

    class object_class_builder
    {
        physics_space& space;
        friend class physics_space;
        inline object_class_builder(physics_space& p, movement::movement_controller* controller, const std::string& name)
            : space(p), controller(controller), name(name)
        {
        }

        std::vector<std::unique_ptr<forces::force>> forces;
        std::vector<std::unique_ptr<render::renderer>> renderers;
        std::vector<void (*)(void*)> deleters;

        std::unique_ptr<movement::movement_controller> controller;

        std::unordered_map<std::string, std::size_t> name2idx;
        std::size_t idx = 0;
        std::string name;

        std::size_t register_type(const char* name, void (*del)(void*));

    public:
        template <typename F, typename... Args>
        constexpr object_class_builder& force(Args&&... args)
        {
            forces.push_back(std::make_unique<F>(std::forward<Args>(args)...));
            return *this;
        }

        constexpr object_class_builder& gravity(double constant) { return force<forces::gravity>(constant); }
        constexpr object_class_builder& const_acc(const vec2d& v) { return force<forces::const_acc>(v); }
        constexpr object_class_builder& const_acc(double x, double y) { return force<forces::const_acc>(vec2d{x, y}); }

        template <typename F, typename... Args>
        constexpr object_class_builder& renderer(Args&&... args)
        {
            renderers.push_back(std::make_unique<F>(slot_allocator(this, &object_class_builder::register_type),
                                                    std::forward<Args>(args)...));
            return *this;
        }

        inline object_class_builder& circle() { return renderer<render::circle_renderer>(); }
        inline object_class_builder& trail(double min_dist) { return renderer<render::trail_renderer>(min_dist); }

        inline object_class_builder& render_vel(double scale)
        {
            return renderer<render::arrow_renderer<render::VEL>>(scale);
        }

        inline object_class_builder& render_acc(double scale)
        {
            return renderer<render::arrow_renderer<render::ACC>>(scale);
        }

        void build();
    };
} // namespace phy

#endif
