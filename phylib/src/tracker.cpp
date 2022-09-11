#include <fmt/ranges.h>
#include <tracker.h>

namespace phy
{
    tracker::tracker(double sample_ticks, std::size_t sample_n, double width)
        : sample_n(sample_n), sample_ticks(sample_ticks), width(width)
    {
    }

    void tracker::handle_update(physics_space& space, double dt)
    {
        ticks += dt;
        if (ticks > sample_ticks)
        {
            std::size_t index = 0;
            for (const auto& i : objects)
            {
                double value = 0;
                switch (i.type)
                {
                case statspec_types::POS:
                    value = i.obj.get_pos().magnitude();
                    break;
                case statspec_types::VEL:
                    value = i.obj.get_vel().magnitude();
                    break;
                case statspec_types::MOMENTUM:
                    value = i.obj.get_vel().magnitude() * i.obj.get_mass();
                    break;
                case statspec_types::ACC:
                    value = i.obj.get_acc().magnitude();
                    break;
                case statspec_types::FORCE:
                    value = i.obj.get_acc().magnitude() * i.obj.get_mass();
                    break;
                case statspec_types::POS_X:
                    value = i.obj.get_pos()[0];
                    break;
                case statspec_types::VEL_X:
                    value = i.obj.get_vel()[0];
                    break;
                case statspec_types::MOMENTUM_X:
                    value = i.obj.get_vel()[0] * i.obj.get_mass();
                    break;
                case statspec_types::ACC_X:
                    value = i.obj.get_acc()[0];
                    break;
                case statspec_types::FORCE_X:
                    value = i.obj.get_acc()[0] * i.obj.get_mass();
                    break;
                case statspec_types::POS_Y:
                    value = i.obj.get_pos()[1];
                    break;
                case statspec_types::VEL_Y:
                    value = i.obj.get_vel()[1];
                    break;
                case statspec_types::MOMENTUM_Y:
                    value = i.obj.get_vel()[1] * i.obj.get_mass();
                    break;
                case statspec_types::ACC_Y:
                    value = i.obj.get_acc()[1];
                    break;
                case statspec_types::FORCE_Y:
                    value = i.obj.get_acc()[1] * i.obj.get_mass();
                    break;
                case statspec_types::KE:
                    value = i.obj.get_vel().magnitude() * i.obj.get_vel().magnitude() * 0.5;
                    break;
                }

                buf[index++].push_back(value);
            }
            ticks -= sample_ticks;
        }
    }

    void tracker::handle_render(sf::RenderTarget& rw)
    {
        sf::Vertex* vert = new sf::Vertex[buf.size() == 0 ? 0 : buf[0].size()];

        for (std::size_t i = 0; i < buf.size(); i++)
        {
            for (std::size_t j = 0; j < buf[i].size(); j++)
            {
                vert[j].color = objects[i].c;
                vert[j].position = {(float)(j * width), (float)(100 - buf[i][j] + 20)};
            }

            rw.draw(vert, buf[i].size(), sf::LineStrip);
        }

        delete[] vert;
    }
} // namespace phy
