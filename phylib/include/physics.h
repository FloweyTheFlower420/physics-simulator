#ifndef __PHY_PHYSICS_H__
#define __PHY_PHYSICS_H__
#include "tracker.h"
#include <SFML/Graphics.hpp>
#include <chrono>
#include <logger_ref.h>
#include <memory>
#include <object.h>
#include <special_object.h>
#include <stdexcept>
#include <string>
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

        std::unordered_map<std::string, std::unique_ptr<object_class>> clazz;
        std::vector<std::unique_ptr<object>> objects;
        std::vector<vec2d> forces_cache;
        std::vector<std::unique_ptr<special_object>> special_objects;

        tick_counter<std::chrono::microseconds> tick;
        framerate_counter counter;

        logging::logger_ref ref;
        double subtick_mult;
        std::size_t cycles;

        std::unique_ptr<tracker> t;

    public:
        constexpr double get_tick_mult() const { return subtick_mult; }
        constexpr std::size_t get_cycles() const { return cycles; }
        constexpr void set_tick_mult(double m)
        {
            if (m < 0.0001)
                m = 0.0001;
            subtick_mult = m;
        }

        constexpr void set_cycles(std::size_t n)
        {
            if (n == 0)
                n = 1;
            if (n >= 200000)
                n = 200000;

            cycles = n;
        }

        inline physics_space(sf::RenderWindow& rw, sf::Font& font, double subtick_mult, std::size_t cycles)
            : rw(rw), font(font), tick(), ref("phy_space"), subtick_mult(subtick_mult), cycles(cycles)
        {
        }

        inline void reset() { tick.dt(); }

        void render(const std::string& str = "");

        template <typename T>
        object_class_builder& create_class(const std::string& name)
        {
            return *new object_class_builder(*this, new T(), name);
        }

        template <typename T, typename... Args>
        T* create_special(Args&&... args)
        {
            return (T*)special_objects.emplace_back(std::make_unique<T>(std::forward<Args>(args)...)).get();
        }

        inline bool class_exists(const std::string& name) { return clazz.contains(name); }

        object_builder create_object(const std::string& name, double mass, const named_value_map& m);

        constexpr sf::RenderWindow& with_window() { return rw; }
        constexpr const sf::RenderWindow& with_window() const { return rw; }
        inline tracker* make_tracker(double a, std::size_t b, double c)
        {
            return (t = std::make_unique<tracker>(a, b, c)).get();
        }
    };
} // namespace phy

#endif
