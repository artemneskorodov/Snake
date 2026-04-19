#include <SFML/Graphics.hpp>

#include "view.hh"
#include "model.hh"
#include "graphics_view.hh"

namespace snake
{

namespace
{

constexpr float kCellSize     = 30.f;
constexpr float kHeaderHeight = 80.f;

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

struct TextureSpriteInfo
{
    bool      turning;
    bool      need_flip;
    sf::Angle rotation;
};

TextureSpriteInfo
get_texture_sprite_info( Direction prev_curr,
                         Direction curr_next)
{
    TextureSpriteInfo info{};

    if ( prev_curr == curr_next )
    {
        info.need_flip = false;
        info.turning = false;
        info.rotation = sf::degrees( DirectionToDegrees( prev_curr));
    } else
    {
        info.turning = true;
        info.need_flip = false;
        int prev_curr_degrees = DirectionToDegrees( prev_curr);
        int curr_next_degrees = DirectionToDegrees( curr_next);

        int difference = (360 + curr_next_degrees - prev_curr_degrees) % 360;
        if ( difference == 90 )
        {
            info.rotation = sf::degrees( prev_curr_degrees);
            info.need_flip = false;
        } else if ( difference == 270 )
        {
            info.rotation = sf::degrees( curr_next_degrees + 90);
            info.need_flip = true;
        } else
        {
            throw std::runtime_error{ "Unexpected directions pair"};
        }
    }

    return info;
}

Direction
vector_to_direction( Point point)
{
    bool is_valid = true;
    if ( point.x == 1 || point.x == -1 )
    {
        if ( point.y != 0 )
        {
            is_valid = false;
        }
    } else
    {
        if ( point.x != 0 )
        {
            is_valid = false;
        }
        if ( point.y != 1 && point.y != -1 )
        {
            is_valid = false;
        }
    }
    if ( !is_valid )
    {
        throw std::runtime_error{ "Expected to call " + std::string( __FUNCTION__) + " only for "
                                  "difference of neighbour points"};
    }

    if ( point.x == 1 )
    {
        return Direction::RIGHT;
    } else if ( point.x == -1 )
    {
        return Direction::LEFT;
    } else if ( point.y == 1 )
    {
        return Direction::BOTTOM;
    } else
    {
        return Direction::TOP;
    }
}

sf::Sprite
get_textured_sprite( const TextureSpriteInfo& info,
                     const sf::Texture&       straight,
                     const sf::Texture&       turning,
                     Coordinate               x,
                     Coordinate               y)
{
    const sf::Texture* texture = info.turning ? &turning
                                              : &straight;

    sf::Sprite sprite{ *texture};

    auto texture_size = texture->getSize();
    sprite.setOrigin( { texture_size.x / 2.f, texture_size.y / 2.f});

    float flip_mult = info.need_flip ? -1.f : 1.f;

    sprite.setScale( { flip_mult * kCellSize / texture_size.x,
                       kCellSize / texture_size.y});

    sprite.setRotation( info.rotation);

    sprite.setPosition( { x * kCellSize + kCellSize / 2.f,
                          y * kCellSize + kCellSize / 2.f});

    return sprite;
}

} // anonymous namespace

GraphicsView::GraphicsView( uint32_t width,
                            uint32_t height)
 :  window_ { sf::VideoMode{ sf::Vector2u{ width, height}}, "Snake"}
{
    current_window_size_ = { width, height};
}

GraphicsView::~GraphicsView()
{
    window_.close();
}

void
GraphicsView::Render( const Model& model)
{
    window_.clear( sf::Color::Black);
    render_game_field();
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
    for ( const Bone& bone : model.GetBones() )
    {
        if ( !bone.is_alive )
        {
            continue;
        }
        render_bone( bone);
    }
    window_.display();
}

std::pair<Coordinate, Coordinate>
GraphicsView::GetGameFieldSize() const
{
    return { current_window_size_.first  * 0.85 / kCellSize,
             current_window_size_.second * 0.85 / kCellSize};
}

void
GraphicsView::UpdateEvents()
{
    for ( ; ; )
    {
        std::optional event = window_.pollEvent();
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
            window_.setView( sf::View( visibleArea));
        }
    }
}

void
GraphicsView::render_snake( const Snake& snake)
{
    auto it  = snake.points.cbegin();
    auto end = snake.points.cend();

    Direction dir_from_prev;
    Direction dir_to_next;

    dir_to_next = vector_to_direction( *std::next( it) - *it);
    TextureSpriteInfo info = get_texture_sprite_info( dir_to_next, dir_to_next);

    window_.draw(
        get_textured_sprite(
            info,
            textures_.snake_texture_tail,
            textures_.snake_texture_tail,
            it->x,
            it->y)
    );
    ++it;

    for ( ; it != std::prev( end); ++it )
    {
        dir_from_prev = vector_to_direction( *it - *std::prev( it));
        dir_to_next   = vector_to_direction( *std::next( it) - *it);

        info = get_texture_sprite_info( dir_from_prev, dir_to_next);

        window_.draw(
            get_textured_sprite(
                info,
                textures_.snake_texture_body_straight,
                textures_.snake_texture_body_turning,
                it->x,
                it->y)
        );
    }

    dir_from_prev = vector_to_direction( *it - *std::prev( it));
    dir_to_next   = snake.direction;

    info = get_texture_sprite_info( dir_from_prev, dir_to_next);
    window_.draw(
        get_textured_sprite(
            info,
            textures_.snake_texture_head_straight,
            textures_.snake_texture_head_turning,
            it->x,
            it->y)
    );
}

void
GraphicsView::render_rabbit( const Rabbit& rabbit)
{
    // TODO add texture
    sf::RectangleShape shape{ sf::Vector2f{ kCellSize, kCellSize}};
    shape.setPosition( sf::Vector2f{ rabbit.point.x * kCellSize, rabbit.point.y * kCellSize});
    shape.setFillColor( sf::Color::Blue);
    window_.draw( shape);
}

void
GraphicsView::render_bone( const Bone& bone)
{
    sf::Sprite sprite{ textures_.snake_bone_texture};
    auto texture_size = textures_.snake_bone_texture.getSize();
    sprite.setScale( { kCellSize / texture_size.x, kCellSize / texture_size.y});
    sprite.setPosition( { bone.point.x * kCellSize, bone.point.y * kCellSize});
    window_.draw( sprite);
}

void
GraphicsView::render_game_field()
{
    float width  = static_cast<float>( current_window_size_.first);

    // Drawing game header
    sf::VertexArray header{ sf::PrimitiveType::TriangleFan, 4};

    header[0].position = sf::Vector2f{     0,             0};
    header[1].position = sf::Vector2f{ width,             0};
    header[2].position = sf::Vector2f{ width, kHeaderHeight};
    header[3].position = sf::Vector2f{     0, kHeaderHeight};

    header[0].color = sf::Color{ 0x247a32ff}; // #247a32
    header[1].color = sf::Color{ 0x247a32ff}; // #247a32
    header[2].color = sf::Color{ 0x165320ff}; // #165320
    header[3].color = sf::Color{ 0x165320ff}; // #165320

    window_.draw( header);
}

} // ! namespace snake
