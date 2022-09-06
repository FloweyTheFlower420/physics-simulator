#include "object.h"
#include <SFML/Graphics.hpp>
#include <fmt/core.h>
#include <iostream>
#include <physics.h>
namespace phy
{
    void physics_space::render(const std::string& msg)
    {
        counter.update();
        
        for(std::size_t rcycle = 0; rcycle < cycles; rcycle++)
        {
            double dt = tick.dt() * subtick_mult;
            forces_cache.clear();
            forces_cache.resize(objects.size());

            for (std::size_t i = 0; i < objects.size(); i++)
            {
                for (auto& j : objects)
                    forces_cache[i] += j->apply_force(*objects[i]);
            }

            for (std::size_t i = 0; i < objects.size(); i++)
                objects[i]->update(dt, forces_cache[i]);

            for (auto& i : objects)
                i->step_time();
        }

        for (auto& i : objects)
            rw.draw(*i);

        sf::Text text(fmt::format("FPS={}\n{}", counter.get(), msg), font);

        rw.setView(rw.getDefaultView());
        text.setPosition(0, 0);
        text.scale(0.5, 0.5);
        rw.draw(text);
    }

    object_builder physics_space::create_object(const char* clazz_name, double mass, const named_value_map& m)
    {
        return object_builder(*objects.emplace_back(new object(mass, clazz.at(clazz_name).get(), m)).get());
    }
} // namespace phy
