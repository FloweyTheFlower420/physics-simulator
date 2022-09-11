#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <component/force.h>
#include <component/movement.h>
#include <component/renderers/arrow_renderer.h>
#include <component/renderers/circle_renderer.h>
#include <component/renderers/trail_renderer.h>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger_ostream.h>
#include <logger_ref.h>
#include <logging.h>
#include <object.h>
#include <physics.h>
#include <sstream>
#include <util/builers.h>

sf::Color rgb(uint32_t val) { return sf::Color(val << 8 | 0xff); }

using namespace phy;

bool wait_start(sf::RenderWindow& window, sf::Font& f)
{
    while (window.isOpen())
    {
        window.clear();
        window.draw(
            sf::Text("Keyword:\nKey 'R' -- Start System\nKey 'Q' -- Quit\nMouse Drag -- Move\nScroll -- Zoom", f, 30));
        window.display();

        sf::Event event;
        while (window.waitEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::R:
                    return false;
                case sf::Keyboard::Q:
                    window.close();
                    return true;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
    }

    return false;
}

phy::physics_space create_space(const std::string& file, sf::RenderWindow& rw, sf::Font& f, double subtick_mult,
                                std::size_t cycles);

extern char font_ttf[];
extern unsigned int font_ttf_len;

int start(char* file)
{
    sf::RenderWindow window(sf::VideoMode(1440, 1080), "Physics Sim");
    sf::Font font;
    logging::logger_ref ref("phy");

    ref.info(fmt::format("working directory: {}", std::filesystem::current_path().string()));

    font.loadFromMemory(font_ttf, font_ttf_len);
    if (wait_start(window, font))
        return 0;

    ref.info("starting window");
    physics_space space = create_space(file, window, font, 1, 1);

    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::View v = window.getDefaultView();
    sf::Vector2f view_center = v.getCenter();
    double scale = 1;

    window.setKeyRepeatEnabled(false);
    space.reset();
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Q:
                    window.close();
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::Resized: {
                ref.info("event");
                sf::View v = window.getView();
                v.setSize({static_cast<float>(event.size.width), static_cast<float>(event.size.height)});
                window.setView(v);
            }
            break;
            case sf::Event::MouseButtonPressed:
                switch (event.mouseButton.button)
                {
                case sf::Mouse::Left:
                    view_center = v.getCenter();
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::MouseWheelScrolled:
                scale *= event.mouseWheelScroll.delta == 1 ? 0.5 : 2;
                v.setSize(v.getSize() * (event.mouseWheelScroll.delta == 1 ? 0.5f : 2));
                break;
            default:
                break;
            }
        }

        auto curr_mouse_pos = sf::Mouse::getPosition();
        auto disp = vector_cast<double>(mouse_pos - curr_mouse_pos) * scale;
        mouse_pos = curr_mouse_pos;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            v.move(vector_cast<float>(disp));
        }

        window.setView(v);
        window.clear();
        space.render(fmt::format("scale: {}x | pos: ({}, {})", scale, v.getCenter().x, v.getCenter().y));
        window.display();
    }

    ref.info("closing...");

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << fmt::format("usage: {} [config_filename]", argv[0]);
        exit(-1);
    }

    logging::logger::get_instance().add_transport<logging::cout_transporter>(logging::logger::INFO, true);

    try
    {
        return start(argv[1]);
    }
    catch (std::exception& e)
    {
        logging::logger::get_instance().fatal(e);
    }
}
