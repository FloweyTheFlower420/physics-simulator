#ifndef __PHY_OBJECT_CLASS_H__
#define __PHY_OBJECT_CLASS_H__
#include <util/obj_class_util.h>
#include <component/movement.h>
#include <component/force.h>
#include <component/renderer.h>
#include <memory>
#include <unordered_map>

namespace phy
{
    class object_class_builder;
    class object;

    class object_class
    {
        std::vector<std::unique_ptr<forces::force>> forces;
        std::vector<std::unique_ptr<render::renderer>> renderers;
        std::vector<void (*)(void*)> deleters;
        index_map named_vmap;
        std::unique_ptr<movement::movement_controller> controller;

        friend class object_class_builder;
        friend class object;

        object_class() = default;
        
        void init_object(object& obj, const named_value_map& vmap) const;
        void destroy_object(object& obj) const;
    public:
        void set_key(object& obj, const char* s) const;
        constexpr std::size_t vmap_size() const { return deleters.size(); }
    };
}

#endif
