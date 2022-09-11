#include <memory>
#include <object_class.h>
#include <physics.h>
#include <util/builers.h>
namespace phy
{
    std::size_t object_class_builder::register_type(const char* name, void (*del)(void*))
    {
        if (!name)
        {
            deleters.push_back(del);
            return idx++;
        }

        if (name2idx.find(name) != name2idx.end())
            return name2idx.at(name);

        std::size_t ret = idx++;
        deleters.push_back(del);
        name2idx[name] = ret;
        return ret;
    }

    void object_class_builder::build()
    {
        std::unique_ptr<object_class> clazz(new object_class());
        clazz->forces = std::move(forces);
        clazz->renderers = std::move(renderers);
        clazz->deleters = std::move(deleters);
        clazz->named_vmap = std::move(name2idx);
        clazz->controller = std::move(controller);

        space.clazz[name] = std::move(clazz);

        // !!!
        delete this;
    }
} // namespace phy
