#include <object.h>
#include <object_class.h>

namespace phy
{
    void object_class::init_object(object& obj, const named_value_map& vmap) const
    {
        obj.vmap.resize(vmap_size());
        for (const auto& i : renderers)
            i->init(obj, vmap);
    }

    void object_class::destroy_object(object& obj) const
    {
        for (std::size_t i = 0; i < obj.vmap.size(); i++)
            deleters[i](obj.vmap[i]);
    }
} // namespace phy
