#include <SFML/Graphics.hpp>

#include "view.hh"
#include "model.hh"
#include "graphics_view.hh"

namespace snake
{

GraphicsView::GraphicsView( uint32_t width,
                            uint32_t height)
 :  window{ sf::VideoMode{ sf::Vector2u{ width, height}}, "Snake"}
{
    current_window_size_ = { width, height};
}

GraphicsView::~GraphicsView()
{
    window.close();
}

void
GraphicsView::Render( const Model& model)
{
    window.clear( sf::Color::Black);
    for ( const Snake& snake : model.GetSnakes() )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
        for ( const Point& point : snake.points )
        {
            sf::RectangleShape shape{ sf::Vector2f{ 10.f, 10.f}};
            shape.setPosition( sf::Vector2f{ point.x * 10.f, point.y * 10.f});
            shape.setFillColor( sf::Color::Green);
            window.draw( shape);
        }
    }
    for ( const Rabbit& rabbit : model.GetRabbits() )
    {
        if ( !rabbit.is_alive )
        {
            continue;
        }
        sf::RectangleShape shape{ sf::Vector2f{ 10.f, 10.f}};
        shape.setPosition( sf::Vector2f{ rabbit.point.x * 10.f, rabbit.point.y * 10.f});
        shape.setFillColor( sf::Color::Blue);
        window.draw( shape);
    }
    window.display();
}

std::pair<Coordinate, Coordinate>
GraphicsView::GetGameFieldSize() const
{
    return { current_window_size_.first * 0.85 / 10, current_window_size_.second * 0.85 / 10};
}

void
GraphicsView::UpdateEvents()
{
    for ( ; ; )
    {
        std::optional event = window.pollEvent();
        if ( !event.has_value() )
        {
            break;
        }

        if ( event->is<sf::Event::Closed>() )
        {
            events_.emplace_back( Event::KEY_PRESSED_EXIT);
        } else if ( event->is<sf::Event::KeyPressed>() )
        {
            const sf::Event::KeyPressed *key = event->getIf<sf::Event::KeyPressed>();
            switch ( key->scancode )
            {
                case sf::Keyboard::Scancode::Left:
                {
                    events_.emplace_back( Event{ 0, Event::KEY_PRESSED_PLAYER_LEFT});
                    break;
                }
                case sf::Keyboard::Scancode::Down:
                {
                    events_.emplace_back( Event{ 0, Event::KEY_PRESSED_PLAYER_BOTTOM});
                    break;
                }
                case sf::Keyboard::Scancode::Right:
                {
                    events_.emplace_back( Event{ 0, Event::KEY_PRESSED_PLAYER_RIGHT});
                    break;
                }
                case sf::Keyboard::Scancode::Up:
                {
                    events_.emplace_back( Event{ 0, Event::KEY_PRESSED_PLAYER_TOP});
                    break;
                }
                case sf::Keyboard::Scancode::A:
                {
                    events_.emplace_back( Event{ 1, Event::KEY_PRESSED_PLAYER_LEFT});
                    break;
                }
                case sf::Keyboard::Scancode::S:
                {
                    events_.emplace_back( Event{ 1, Event::KEY_PRESSED_PLAYER_BOTTOM});
                    break;
                }
                case sf::Keyboard::Scancode::D:
                {
                    events_.emplace_back( Event{ 1, Event::KEY_PRESSED_PLAYER_LEFT});
                    break;
                }
                case sf::Keyboard::Scancode::W:
                {
                    events_.emplace_back( Event{ 1, Event::KEY_PRESSED_PLAYER_TOP});
                    break;
                }
                case sf::Keyboard::Scancode::Q:
                {
                    events_.emplace_back( Event::KEY_PRESSED_EXIT);
                    break;
                }
                default:
                {
                    break;
                }
            }
        } else if ( event->is<sf::Event::Resized>() )
        {
            const sf::Event::Resized *resize = event->getIf<sf::Event::Resized>();
            events_.emplace_back( Event::WINDOW_SIZE_CHANGED);
            current_window_size_ = { resize->size.x, resize->size.y};
            sf::FloatRect visibleArea( { 0, 0},
                                       { static_cast<float>( resize->size.x),
                                         static_cast<float>( resize->size.y)});
            window.setView( sf::View( visibleArea));
        }
    }
}

} // ! namespace snake
