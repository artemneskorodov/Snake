#include <SFML/Graphics.hpp>

#include "view.hh"
#include "model.hh"
#include "graphics_view.hh"

namespace snake
{

namespace
{

constexpr float kCellSize = 30.f;

const std::unordered_map<sf::Keyboard::Scancode, Event> kKeyInfo{
    {sf::Keyboard::Scancode::Left,  Event{ 0, Event::KEY_PRESSED_PLAYER_LEFT   }},
    {sf::Keyboard::Scancode::Down,  Event{ 0, Event::KEY_PRESSED_PLAYER_BOTTOM }},
    {sf::Keyboard::Scancode::Right, Event{ 0, Event::KEY_PRESSED_PLAYER_RIGHT  }},
    {sf::Keyboard::Scancode::Up,    Event{ 0, Event::KEY_PRESSED_PLAYER_TOP    }},
    {sf::Keyboard::Scancode::A,     Event{ 1, Event::KEY_PRESSED_PLAYER_LEFT   }},
    {sf::Keyboard::Scancode::S,     Event{ 1, Event::KEY_PRESSED_PLAYER_BOTTOM }},
    {sf::Keyboard::Scancode::D,     Event{ 1, Event::KEY_PRESSED_PLAYER_RIGHT  }},
    {sf::Keyboard::Scancode::W,     Event{ 1, Event::KEY_PRESSED_PLAYER_TOP    }},
    {sf::Keyboard::Scancode::Q,     Event::KEY_PRESSED_EXIT                     },
};

} // anonymous namespace

GraphicsView::GraphicsView( uint32_t width,
                            uint32_t height)
 :  window{ sf::VideoMode{ sf::Vector2u{ width, height}}, "Snake"},
    forward_snake_texture_{ sf::Texture( "resources/forward.png")},
    rotate_snake_texture_{ sf::Texture( "resources/rotate.png")}
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
        render_snake( snake);
    }
    for ( const Rabbit& rabbit : model.GetRabbits() )
    {
        if ( !rabbit.is_alive )
        {
            continue;
        }
        render_rabbit( rabbit);
    }
    window.display();
}

std::pair<Coordinate, Coordinate>
GraphicsView::GetGameFieldSize() const
{
    return { current_window_size_.first * 0.85 / kCellSize, current_window_size_.second * 0.85 / kCellSize};
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
            if ( kKeyInfo.find( key->scancode) != kKeyInfo.end() )
            {
                events_.emplace_back( kKeyInfo.at( key->scancode));
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

void
GraphicsView::render_snake( const Snake& snake)
{
    auto it  = snake.points.cbegin();
    auto end = snake.points.cend();

    // TODO tail
    sf::RectangleShape shape{ sf::Vector2f{ kCellSize, kCellSize}};
    shape.setPosition( sf::Vector2f{ it->x * kCellSize, it->y * kCellSize});
    shape.setFillColor( sf::Color::Yellow);
    window.draw( shape);

    if ( it == end )
    {
        // TODO make separate texture or make the smallest size equal 2
        return ;
    }

    ++it;

    for ( ; it != std::prev( end); ++it )
    {
        Point dir_from_prev = *it - *std::prev( it);
        Point dir_to_next   = *std::next( it) - * it;

        const sf::Texture *texture = nullptr;
        sf::Angle rotation;
        bool need_flip = false;

        if ( dir_from_prev == dir_to_next )
        {
            texture = &forward_snake_texture_;
            if ( dir_from_prev.x == 1 )
            {
                rotation = sf::degrees( 90);
            } else if ( dir_from_prev.x == -1 )
            {
                rotation = sf::degrees( 270);
            } else if ( dir_from_prev.y == 1 )
            {
                rotation = sf::degrees( 0);
            } else
            {
                rotation = sf::degrees( 180);
            }
        } else
        {
            texture = &rotate_snake_texture_;
            if ( dir_to_next == Point{1, 0} && dir_from_prev == Point{ 0, -1})
            {
                rotation = sf::degrees( 0);
            } else if ( dir_to_next == Point{ 0, 1} && dir_from_prev == Point{ -1, 0})
            {
                rotation = sf::degrees( 0 - 90);
                need_flip = true;
            } else if ( dir_to_next == Point{ 0, 1} && dir_from_prev == Point{ 1, 0})
            {
                rotation = sf::degrees( 90);
            } else if ( dir_to_next == Point{ -1, 0} && dir_from_prev == Point{ 0, -1})
            {
                rotation = sf::degrees( 90 - 90);
                need_flip = true;
            } else if ( dir_to_next == Point{ -1, 0} && dir_from_prev == Point{ 0, 1})
            {
                rotation = sf::degrees( 180);
            } else if ( dir_to_next == Point{ 0, -1} && dir_from_prev == Point{ 1, 0})
            {
                rotation = sf::degrees( 180 - 90);
                need_flip = true;
            } else if ( dir_to_next == Point{ 0, -1} && dir_from_prev == Point{ -1, 0})
            {
                rotation = sf::degrees( 270);
            } else if ( dir_to_next == Point{ 1, 0} && dir_from_prev == Point{ 0, 1})
            {
                rotation = sf::degrees( 270 - 90);
                need_flip = true;
            }
        }

        sf::Sprite sprite{ *texture};
        auto texture_size = texture->getSize();
        sprite.setOrigin( { texture_size.x / 2.f, texture_size.y / 2.f});
        sprite.setPosition( sf::Vector2f{ it->x * kCellSize + kCellSize / 2.f,
                                                        it->y * kCellSize + kCellSize / 2.f});
        float flip_mult = need_flip ? -1.f : 1.f;
        sprite.setScale( { flip_mult * kCellSize / texture_size.x, kCellSize / texture_size.y});
        sprite.setRotation( rotation);
        window.draw( sprite);
    }

    // TODO head
    shape.setPosition( sf::Vector2f{ it->x * kCellSize, it->y * kCellSize});
    shape.setFillColor( sf::Color::Red);
    window.draw( shape);
}

void
GraphicsView::render_rabbit( const Rabbit& rabbit)
{
    // TODO add texture
    sf::RectangleShape shape{ sf::Vector2f{ kCellSize, kCellSize}};
    shape.setPosition( sf::Vector2f{ rabbit.point.x * kCellSize, rabbit.point.y * kCellSize});
    shape.setFillColor( sf::Color::Blue);
    window.draw( shape);
}

} // ! namespace snake
