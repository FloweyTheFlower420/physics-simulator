#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
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

bool wait_start(sf::RenderWindow& window)
{
    while (window.isOpen())
    {
        window.clear();
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

int start()
{
    sf::RenderWindow window(sf::VideoMode(1440, 1080), "Example Phy Sim");
    sf::Font font;
    logging::logger_ref ref("phy");

    ref.info(fmt::format("working directory: {}", std::filesystem::current_path().string()));

    if (!font.loadFromFile("font.ttf"))
        throw std::runtime_error("unable to open font");

    if (wait_start(window))
        return 0;

    ref.info("starting window");
    physics_space space(window, font, 0.25, 8000);

    space.create_class<movement::default_controller>("test")
        .trail(10)
        .render_acc(5)
        .render_vel(2)
        .circle()
        .gravity(120000)
        .build();

    /*space.create_object("test", 1, {
        render::trail_renderer::COLOR_KEY(rgb(0xaa0000)),
        render::arrow_renderer<phy::render::VEL>::COLOR_KEY(rgb(0xaaaa00)),
        render::arrow_renderer<phy::render::ACC>::COLOR_KEY(rgb(0x00aaaa)),
        render::circle_renderer::COLOR_KEY(rgb(0xee1111)), render::circle_renderer::RADIUS_KEY(13)
    }).pos(600, 600).momentum(50, -30);*/

    space.create_object("test", 5, {
        render::trail_renderer::COLOR_KEY(rgb(0x00aa00)),
        render::arrow_renderer<phy::render::VEL>::COLOR_KEY(rgb(0xaaaa00)),
        render::arrow_renderer<phy::render::ACC>::COLOR_KEY(rgb(0x00aaaa)),
        render::circle_renderer::COLOR_KEY(rgb(0x11ee11)),
        render::circle_renderer::RADIUS_KEY(13),
    }).pos(800, 800).momentum(-50, 10);

    space.create_object("test", 3, {
        render::trail_renderer::COLOR_KEY(rgb(0x0000aa)),
        render::arrow_renderer<phy::render::VEL>::COLOR_KEY(rgb(0xaaaa00)),
        render::arrow_renderer<phy::render::ACC>::COLOR_KEY(rgb(0x00aaaa)),
        render::circle_renderer::COLOR_KEY(rgb(0x1111ee)), render::circle_renderer::RADIUS_KEY(13)
    }).pos(900, 200).momentum(0, 20);

    space.create_object("test", 0.001, {
        render::trail_renderer::COLOR_KEY(rgb(0xdddddd)),
        render::arrow_renderer<phy::render::VEL>::COLOR_KEY(rgb(0xaaaa00)),
        render::arrow_renderer<phy::render::ACC>::COLOR_KEY(rgb(0x00aaaa)),
        render::circle_renderer::COLOR_KEY(rgb(0xffffff)), render::circle_renderer::RADIUS_KEY(3)
    }).pos(1050, 150).momentum(0, .05);

    space.create_object("test", 0.001, {
        render::trail_renderer::COLOR_KEY(rgb(0xdddddd)),
        render::arrow_renderer<phy::render::VEL>::COLOR_KEY(rgb(0xaaaa00)),
        render::arrow_renderer<phy::render::ACC>::COLOR_KEY(rgb(0x00aaaa)),
        render::circle_renderer::COLOR_KEY(rgb(0xffffff)), render::circle_renderer::RADIUS_KEY(3)
    }).pos(750, 250).momentum(0, -.05);

    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
    sf::View v = window.getDefaultView();
    sf::Vector2f view_center = v.getCenter();
    double scale = 1;

    window.setKeyRepeatEnabled(false);

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
            case sf::Event::Resized:
                {
                    ref.info("event");
                    sf::View v = window.getView();
                    v.setSize({
                                        static_cast<float>(event.size.width),
                                        static_cast<float>(event.size.height)
                    });
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
                scale *= event.mouseWheelScroll.delta == -1 ? 0.5 : 2;
                v.setSize(v.getSize() * (event.mouseWheelScroll.delta == -1 ? 0.5f : 2));
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

int main()
{
    logging::logger::get_instance().add_transport<logging::cout_transporter>(logging::logger::INFO, true);

    try
    {
        return start();
    }
    catch (std::exception& e)
    {
        logging::logger::get_instance().fatal(e);
    }
}
