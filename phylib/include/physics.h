#ifndef __PHY_PHYSICS_H__
#define __PHY_PHYSICS_H__
#include <SFML/Graphics.hpp>
#include <chrono>
#include <logger_ref.h>
#include <memory>
#include <object.h>
#include <stdexcept>
#include <string_view>
#include <util/builers.h>
#include <util/chrono_util.h>
#include <util/obj_class_util.h>
#include <util/perf_counter.h>
namespace phy
{
    class physics_space
    {
        friend class object_class_builder;

        sf::RenderWindow& rw;
        sf::Font& font;

        std::unordered_map<std::string_view, std::unique_ptr<object_class>> clazz;
        std::vector<std::unique_ptr<object>> objects;
        std::vector<vec2d> forces_cache;
        tick_counter<std::chrono::microseconds> tick;
        framerate_counter counter;

        logging::logger_ref ref;
        double subtick_mult;
        std::size_t cycles;

    public:
        inline physics_space(sf::RenderWindow& rw, sf::Font& font, double subtick_mult, std::size_t cycles)
            : rw(rw), font(font), tick(), ref("phy_space"), cycles(cycles), subtick_mult(subtick_mult)
        {
        }

        void render(const std::string& str = "");

        template <typename T>
        object_class_builder& create_class(const char* name)
        {
            return *new object_class_builder(*this, new T(), name);
        }

        object_builder create_object(const char* clazz_name, double mass, const named_value_map& m);

        constexpr sf::RenderWindow& with_window() { return rw; }
        constexpr const sf::RenderWindow& with_window() const { return rw; }
    };
} // namespace phy

#endif
