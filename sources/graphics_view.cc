#include <SFML/Graphics.hpp>

#include "view.hh"
#include "model.hh"
#include "graphics_view.hh"
#include "colors.hh"

namespace snake
{

namespace
{

constexpr float kCellSize         = 30.f;
constexpr float kHeaderHeight     = 40.f;
constexpr float kFooterHeight     = 80.f;

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

const std::unordered_map<sf::Keyboard::Scancode, MenuEvent> kMenuKeyInfo{
    {sf::Keyboard::Scancode::Left,      MenuEvent::KEY_PRESSED_ARROW_LEFT  },
    {sf::Keyboard::Scancode::Down,      MenuEvent::KEY_PRESSED_ARROW_DOWN  },
    {sf::Keyboard::Scancode::Right,     MenuEvent::KEY_PRESSED_ARROW_RIGHT },
    {sf::Keyboard::Scancode::Up,        MenuEvent::KEY_PRESSED_ARROW_UP    },
    {sf::Keyboard::Scancode::Enter,     MenuEvent::KEY_PRESSED_ENTER       },
    {sf::Keyboard::Scancode::Backspace, MenuEvent::BACKSPACE               },
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
                     const sf::Texture&       turning)
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

    return sprite;
}

//
// Colors constants
//

using colors::operator ""_c;

constexpr colors::Color kColorGameHeaderGradientTop    = "#247a32"_c;
constexpr colors::Color kColorGameHeaderGradientBottom = "#165320"_c;
constexpr colors::Color kColorGameHeaderText           = "#a0a0a0"_c;
constexpr colors::Color kColorGameFieldFirst           = "#3bbb55"_c;
constexpr colors::Color kColorGameFieldSecond          = "#43d15f"_c;
constexpr colors::Color kColorGameFieldPadding         = "#a0a0a0"_c;
constexpr colors::Color kColorGameFooterGradientTop    = "#247a32"_c;
constexpr colors::Color kColorGameFooterGradientBottom = "#165320"_c;
constexpr colors::Color kColorFooterDivider            = "#0e3414"_c;

constexpr float    kMenuWidth          = 0.8f;
constexpr float    kMenuElementHeight  = 50.f;
constexpr float    kMenuDefaultOffsetY = 0.1f;
constexpr unsigned kMenuCharacterSize  = 15;

constexpr colors::Color kColorMenuActive         = "#bcffa4"_c;
constexpr colors::Color kColorMenuInactive       = "#b6c9b7"_c;
constexpr colors::Color kColorMenuActiveString   = "#ffffff"_c;
constexpr colors::Color kColorMenuInactiveString = "#c8c8c8"_c;
constexpr colors::Color kColorInvalid            = "#676767"_c;

constexpr float    kStatisticsOffsetX     = 10.f;
constexpr unsigned kStatsCharacterSize    = 13;
constexpr float    kSnakeStatsNameWidth   = 80.f;
constexpr float    kSnakeStatsStatusWidth = 45.f;
constexpr float    kSnakeStatsGroupWidth  = 45.f;
constexpr float    kSnakeStatsLengthWidth = 30.f;
constexpr float    kSnakeStatsWidth       = kSnakeStatsNameWidth +
                                            kSnakeStatsStatusWidth +
                                            kSnakeStatsGroupWidth +
                                            kSnakeStatsLengthWidth;
constexpr float    kSnakeStatsHeight      = 20.f;

constexpr unsigned kSnakeNameCharacterSize = 12;

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
    render_game_statistics( model);
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
    return { (current_window_size_.first                                 ) / kCellSize,
             (current_window_size_.second - kHeaderHeight - kFooterHeight) / kCellSize};
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
            events_.push( Event::KEY_PRESSED_EXIT);
        } else if ( event->is<sf::Event::KeyPressed>() )
        {
            const sf::Event::KeyPressed *key = event->getIf<sf::Event::KeyPressed>();
            if ( kKeyInfo.find( key->scancode) != kKeyInfo.end() )
            {
                events_.push( kKeyInfo.at( key->scancode));
            }
        } else if ( event->is<sf::Event::Resized>() )
        {
            const sf::Event::Resized *resize = event->getIf<sf::Event::Resized>();
            events_.push( Event::WINDOW_SIZE_CHANGED);
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

    sf::Sprite sprite = get_textured_sprite(
        info,
        textures_.snake_texture_tail,
        textures_.snake_texture_tail);
    sf::Vector2f position = game_to_sfml( *it) + sf::Vector2f{ kCellSize / 2.f, kCellSize / 2.f};
    sprite.setPosition( position);
    window_.draw( sprite);
    ++it;

    for ( ; it != std::prev( end); ++it )
    {
        dir_from_prev = vector_to_direction( *it - *std::prev( it));
        dir_to_next   = vector_to_direction( *std::next( it) - *it);

        info = get_texture_sprite_info( dir_from_prev, dir_to_next);

        sprite = get_textured_sprite(
            info,
            textures_.snake_texture_body_straight,
            textures_.snake_texture_body_turning);
        position = game_to_sfml( *it) + sf::Vector2f{ kCellSize / 2.f, kCellSize / 2.f};
        sprite.setPosition( position);
        window_.draw( sprite);
    }

    dir_from_prev = vector_to_direction( *it - *std::prev( it));
    dir_to_next   = snake.direction;

    info = get_texture_sprite_info( dir_from_prev, dir_to_next);

    sprite = get_textured_sprite(
        info,
        textures_.snake_texture_head_straight,
        textures_.snake_texture_head_turning);
    position = game_to_sfml( *it) + sf::Vector2f{ kCellSize / 2.f, kCellSize / 2.f};
    sprite.setPosition( position);
    window_.draw( sprite);

    sf::Text snake_name{ textures_.snake_game_text_font, snake.name};
    sf::Color color = snake.color;
    color.a = 255 * 3 / 4; // Transparency
    snake_name.setFillColor( color);
    snake_name.setCharacterSize( kSnakeNameCharacterSize);
    snake_name.setPosition( position + sf::Vector2f{ kCellSize / 2.f, -kCellSize / 2.f});
    window_.draw( snake_name);
}

void
GraphicsView::render_rabbit( const Rabbit& rabbit)
{
    // TODO add texture
    sf::RectangleShape shape{ sf::Vector2f{ kCellSize, kCellSize}};
    shape.setPosition( game_to_sfml( rabbit.point));
    shape.setFillColor( sf::Color::Blue);
    window_.draw( shape);
}

void
GraphicsView::render_bone( const Bone& bone)
{
    sf::Sprite sprite{ textures_.snake_bone_texture};
    auto texture_size = textures_.snake_bone_texture.getSize();
    sprite.setScale( { kCellSize / texture_size.x, kCellSize / texture_size.y});
    sprite.setPosition( game_to_sfml( bone.point));
    window_.draw( sprite);
}

void
GraphicsView::render_game_field()
{
    float width  = static_cast<float>( current_window_size_.first);
    float height = static_cast<float>( current_window_size_.second);
    float game_field_size_x = width;
    float game_field_size_y = height - kHeaderHeight;

    // Drawing game header
    sf::VertexArray header{ sf::PrimitiveType::TriangleFan, 4};

    header[0].position = sf::Vector2f{     0,             0};
    header[1].position = sf::Vector2f{ width,             0};
    header[2].position = sf::Vector2f{ width, kHeaderHeight};
    header[3].position = sf::Vector2f{     0, kHeaderHeight};

    header[0].color = kColorGameHeaderGradientTop;
    header[1].color = kColorGameHeaderGradientTop;
    header[2].color = kColorGameHeaderGradientBottom;
    header[3].color = kColorGameHeaderGradientBottom;

    window_.draw( header);

    // Drawing header text
    sf::Text header_text{ textures_.snake_game_font, "Snake"};
    sf::FloatRect text_size = header_text.getLocalBounds();
    header_text.setOrigin( text_size.getCenter());
    header_text.setPosition( sf::Vector2f{ width / 2.f, kHeaderHeight / 2.f});
    header_text.setFillColor( kColorGameHeaderText);
    window_.draw( header_text);

    // Filling game field
    sf::RectangleShape game_field_filler{ sf::Vector2f{ game_field_size_x, game_field_size_y}};
    game_field_filler.setFillColor( kColorGameFieldPadding);
    game_field_filler.setPosition( sf::Vector2f{ 0, kHeaderHeight});
    window_.draw( game_field_filler);

    // Drawing game field
    auto field_size = GetGameFieldSize();
    unsigned game_size_x = field_size.first;
    unsigned game_size_y = field_size.second;

    sf::VertexArray game_field{ sf::PrimitiveType::Triangles, game_size_x * game_size_y * 6};

    sf::Vector2f offset_top_left     {       0.f,       0.f};
    sf::Vector2f offset_top_right    { kCellSize,       0.f};
    sf::Vector2f offset_bottom_left  {       0.f, kCellSize};
    sf::Vector2f offset_bottom_right { kCellSize, kCellSize};

    for ( unsigned x = 0; x != game_size_x; ++x )
    {
        for ( unsigned y = 0; y != game_size_y; ++y )
        {
            sf::Vertex* square = &game_field[(x + y * game_size_x) * 6];

            sf::Vector2f top_left = game_to_sfml( x, y);

            square[0].position = top_left + offset_top_left;
            square[1].position = top_left + offset_top_right;
            square[2].position = top_left + offset_bottom_left;

            square[3].position = top_left + offset_top_right;
            square[4].position = top_left + offset_bottom_left;
            square[5].position = top_left + offset_bottom_right;

            sf::Color color = ((x + y) % 2 == 0) ? kColorGameFieldFirst
                                                 : kColorGameFieldSecond;
            for ( int i = 0; i != 6; ++i )
            {
                square[i].color = color;
            }
        }
    }

    window_.draw( game_field);

    // Drawing game footer
    sf::VertexArray footer{ sf::PrimitiveType::TriangleFan, 4};

    footer[0].position = sf::Vector2f{     0, height - kFooterHeight};
    footer[1].position = sf::Vector2f{ width, height - kFooterHeight};
    footer[2].position = sf::Vector2f{ width,                 height};
    footer[3].position = sf::Vector2f{     0,                 height};

    footer[0].color = kColorGameFooterGradientTop;
    footer[1].color = kColorGameFooterGradientTop;
    footer[2].color = kColorGameFooterGradientBottom;
    footer[3].color = kColorGameFooterGradientBottom;

    window_.draw( footer);
}

constexpr inline sf::Vector2f
GraphicsView::game_to_sfml( Coordinate x,
                            Coordinate y)
{
    auto field_size = GetGameFieldSize();
    float game_padding_x = (current_window_size_.first  - field_size.first  * kCellSize) / 2.f;
    float game_padding_y = (current_window_size_.second - kHeaderHeight - kFooterHeight -
                            field_size.second * kCellSize) / 2.f;

    return sf::Vector2f{ static_cast<float>( x) * kCellSize + game_padding_x,
                         static_cast<float>( y) * kCellSize + game_padding_y + kHeaderHeight};
}

constexpr inline sf::Vector2f
GraphicsView::game_to_sfml( const Point& point)
{
    return game_to_sfml( point.x, point.y);
}

void
GraphicsView::RenderMenu( const settings::Menu& settings)
{
    float window_height = static_cast<float>( current_window_size_.second);
    float position = window_height * kMenuDefaultOffsetY;

    window_.clear( sf::Color::Black);
    for ( const settings::MenuElement& element : settings.GetMenu() )
    {
        if ( std::holds_alternative<settings::Button>( element.element) )
        {
            render_menu_button( element, position);
        } else if ( std::holds_alternative<settings::SnakesList>( element.element) )
        {
            render_menu_snakes_list( element, position);
        }
    }
    window_.display();
}

void
GraphicsView::render_menu_button( const settings::MenuElement& menu_elem,
                                  float&                       offset_y)
{
    float window_width  = static_cast<float>( current_window_size_.first);

    float width = window_width * kMenuWidth;
    float x = (window_width - width) / 2.f;
    float y = offset_y;
    float height = kMenuElementHeight * 0.9f;

    float text_offset_x = x + width / 10.f;

    sf::RectangleShape box{ {width, height}};
    box.setPosition( { x, y});
    box.setFillColor( sf::Color::Black);

    sf::Text text{ textures_.snake_game_text_font, menu_elem.name, kMenuCharacterSize};
    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin( { 0, text_rect.size.y / 2.f});
    text.setPosition( { text_offset_x, y + height / 2.f});

    if ( menu_elem.is_active )
    {
        box.setOutlineColor( kColorMenuActive);
        box.setOutlineThickness( 3.f);
        text.setFillColor( kColorMenuActive);
        text.setStyle( text.Bold);
    } else
    {
        box.setOutlineColor( kColorMenuInactive);
        box.setOutlineThickness( 2.f);
        text.setFillColor( kColorMenuInactive);
        text.setStyle( text.Regular);
    }

    window_.draw( box);
    window_.draw( text);

    offset_y += kMenuElementHeight;
}

void
GraphicsView::render_menu_snakes_list( const settings::MenuElement& menu_elem,
                                       float&                       offset_y)
{
    float window_width  = static_cast<float>( current_window_size_.first);
    const settings::SnakesList& snakes_list = std::get<settings::SnakesList>( menu_elem.element);

    float width = window_width * kMenuWidth;
    float x = (window_width - width) / 2.f;
    float y = offset_y;
    float height = kMenuElementHeight * 0.9f;

    float text_offset_x = x + width / 10.f;

    sf::RectangleShape box{ {width, height}};
    box.setPosition( { x, y});
    box.setOutlineThickness( 1.f);
    box.setOutlineColor( sf::Color::Blue);
    box.setFillColor( sf::Color::Black);

    sf::Text text{ textures_.snake_game_text_font, menu_elem.name, kMenuCharacterSize};
    sf::FloatRect text_rect = text.getLocalBounds();
    text.setOrigin( { 0, text_rect.size.y / 2.f});
    text.setPosition( { text_offset_x, y + height / 2.f});

    if ( menu_elem.is_active && snakes_list.active == snakes_list.kNoActive )
    {
        box.setOutlineColor( kColorMenuActive);
        box.setOutlineThickness( 3.f);
        text.setFillColor( kColorMenuActive);
        text.setStyle( text.Bold);
    } else
    {
        box.setOutlineColor( kColorMenuInactive);
        box.setOutlineThickness( 2.f);
        text.setFillColor( kColorMenuInactive);
        text.setStyle( text.Regular);
    }

    window_.draw( box);
    window_.draw( text);

    y += kMenuElementHeight;

    x += width / 10.f;
    width *= 0.9f;

    float text_offset_x_name = x + width / 10.f;
    float text_offset_x_color = x + width * 0.75f;

    box.setSize( { width, height});

    for ( const settings::SnakeSetting& snake : snakes_list.snakes )
    {
        if ( snake.is_active )
        {
            box.setOutlineColor( kColorMenuActive);
            box.setOutlineThickness( 3.f);
        } else
        {
            box.setOutlineColor( kColorMenuInactive);
            box.setOutlineThickness( 2.f);
        }
        box.setPosition( { x, y});
        window_.draw( box);

        text.setString( "Name: " + snake.name);
        text_rect = text.getLocalBounds();
        text.setOrigin( { 0, text_rect.size.y / 2.f});
        text.setPosition( { text_offset_x_name, y + height / 2.f});
        if ( snake.is_active )
        {
            if ( snake.active == settings::SnakeSetting::Active::NAME )
            {
                text.setFillColor( kColorMenuActiveString);
                text.setStyle( text.Bold);
            } else
            {
                text.setFillColor( kColorMenuInactiveString);
                text.setStyle( text.Regular);
            }
        } else
        {
            text.setFillColor( kColorMenuInactiveString);
            text.setStyle( text.Regular);
        }
        window_.draw( text);

        text.setString( "Color: ");
        if ( snake.is_active )
        {
            if ( snake.active == settings::SnakeSetting::Active::COLOR )
            {
                text.setFillColor( kColorMenuActiveString);
                text.setStyle( text.Bold);
            } else
            {
                text.setFillColor( kColorMenuInactiveString);
                text.setStyle( text.Regular);
            }
        } else
        {
            text.setFillColor( kColorMenuInactiveString);
            text.setStyle( text.Regular);
        }
        text_rect = text.getLocalBounds();
        text.setOrigin( { 0, text_rect.size.y / 2.f});
        text.setPosition( { text_offset_x_color, y + height / 2.f});
        window_.draw( text);

        float color_string_width = text_rect.size.x;
        text.setString( snake.color);
        text_rect = text.getLocalBounds();
        text.setOrigin( { 0, text_rect.size.y / 2.f});
        text.setPosition( { text_offset_x_color + color_string_width, y + height / 2.f});
        if ( colors::IsValidColor( snake.color) )
        {
            text.setFillColor( colors::Color{ snake.color});
        } else
        {
            text.setFillColor( kColorInvalid);
        }
        text.setStyle( text.Bold);
        window_.draw( text);

        y += kMenuElementHeight;
    }

    offset_y = y;
}

void
GraphicsView::UpdateMenuEvents()
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
            menu_events_.push( MenuEvent::EXIT);
        } else if ( event->is<sf::Event::TextEntered>() )
        {
            const sf::Event::TextEntered* text = event->getIf<sf::Event::TextEntered>();
            if ( std::isprint( text->unicode) )
            {
                menu_events_.push( static_cast<MenuEvent>( text->unicode));
            }
        } else if ( event->is<sf::Event::KeyPressed>() )
        {
            const sf::Event::KeyPressed *key = event->getIf<sf::Event::KeyPressed>();
            if ( kMenuKeyInfo.find( key->scancode) != kMenuKeyInfo.end() )
            {
                menu_events_.push( kMenuKeyInfo.at( key->scancode));
            }
        } else if ( event->is<sf::Event::Resized>() )
        {
            const sf::Event::Resized *resize = event->getIf<sf::Event::Resized>();
            current_window_size_ = { resize->size.x, resize->size.y};
            sf::FloatRect visibleArea( { 0, 0},
                                       { static_cast<float>( resize->size.x),
                                         static_cast<float>( resize->size.y)});
            window_.setView( sf::View( visibleArea));
        }
    }
}

void
GraphicsView::render_game_statistics( const Model& model)
{
    Statistics statistics = model.GetGameStatistics();

    float width  = static_cast<float>( current_window_size_.first);
    float height = static_cast<float>( current_window_size_.second);

    float x = kStatisticsOffsetX;
    float y = height - kFooterHeight;

    x = draw_group_stats( statistics.human, x, y, "Human Snakes");
    x = draw_group_stats( statistics.dumb,  x, y, "Dumb Snakes");
    x = draw_group_stats( statistics.smart, x, y, "Smart Snakes");

    float state_offset_x = x;
    while ( state_offset_x + kSnakeStatsWidth < width )
    {
        state_offset_x += kSnakeStatsWidth;

        sf::VertexArray divider{ sf::PrimitiveType::Lines, 2};
        divider[0].position = sf::Vector2f{ state_offset_x, y};
        divider[1].position = sf::Vector2f{ state_offset_x, y + kFooterHeight};
        divider[0].color = kColorFooterDivider;
        divider[1].color = kColorFooterDivider;
        window_.draw( divider);
    }

    state_offset_x = x;
    for ( const Snake& snake : model.GetSnakes() )
    {
        draw_snake_stats( snake, state_offset_x, y, model.GetSnakeGroup( snake.id));
        state_offset_x += kSnakeStatsWidth;
        if ( state_offset_x + kSnakeStatsWidth > width )
        {
            state_offset_x = x;
            y += kSnakeStatsHeight;
            if ( y > height )
            {
                break;
            }
        }
    }
}

float
GraphicsView::draw_group_stats( const SnakeGroupStatistics& stats,
                                float                       x,
                                float                       y,
                                const std::string&          name)
{
    if ( stats.alive == 0 && stats.dead == 0 )
    {
        return x;
    }

    sf::Text text{ textures_.snake_game_text_font, name};
    text.setPosition( { x, y});
    text.setCharacterSize( kStatsCharacterSize);
    window_.draw( text);

    float text_height = text.getLocalBounds().size.y;
    float text_offset_y = text_height * 1.5f;

    text.setString( "Alive:");
    text.setPosition( { x, y + text_offset_y});
    window_.draw( text);

    text.setString( "Dead:");
    text.setPosition( { x, y + text_offset_y * 2.f});
    window_.draw( text);

    text.setString( "Total length: ");
    text.setPosition( { x, y + text_offset_y * 3.f});
    window_.draw( text);

    float text_width = text.getLocalBounds().size.x;
    float numbers_offset_x = text_width * 1.1f;

    float max_x = 0.f;

    text.setString( std::to_string( stats.alive));
    text.setPosition( { x + numbers_offset_x, y + text_offset_y});
    window_.draw( text);
    float current_x = x + numbers_offset_x + text.getLocalBounds().size.x;
    if ( current_x > max_x )
    {
        max_x = current_x;
    }

    text.setString( std::to_string( stats.dead));
    text.setPosition( { x + numbers_offset_x, y + text_offset_y * 2.f});
    window_.draw( text);
    current_x = x + numbers_offset_x + text.getLocalBounds().size.x;
    if ( current_x > max_x )
    {
        max_x = current_x;
    }

    text.setString( std::to_string( stats.total_length));
    text.setPosition( { x + numbers_offset_x, y + text_offset_y * 3.f});
    window_.draw( text);
    current_x = x + numbers_offset_x + text.getLocalBounds().size.x;
    if ( current_x > max_x )
    {
        max_x = current_x;
    }

    sf::VertexArray divider{ sf::PrimitiveType::Lines, 2};
    divider[0].position = sf::Vector2f{ max_x + kStatisticsOffsetX / 2.f, y};
    divider[1].position = sf::Vector2f{ max_x + kStatisticsOffsetX / 2.f, y + kFooterHeight};
    divider[0].color = kColorFooterDivider;
    divider[1].color = kColorFooterDivider;
    window_.draw( divider);

    return max_x + kStatisticsOffsetX;
}

void
GraphicsView::draw_snake_stats( const Snake& snake,
                                float        x,
                                float        y,
                                SnakeGroup   group)
{
    sf::Text text{ textures_.snake_game_text_font, snake.name};
    text.setCharacterSize( kStatsCharacterSize);
    text.setPosition( { x, y});
    text.setFillColor( snake.color);
    window_.draw( text);

    text.setFillColor( "#ffffff"_c);
    text.setString( snake.is_alive ? "(alive)" : "(dead)");
    text.setPosition( { x + kSnakeStatsNameWidth, y});
    window_.draw( text);

    switch ( group )
    {
        case SnakeGroup::HUMAN:
        {
            text.setString( "human");
            break;
        }
        case SnakeGroup::DUMB:
        {
            text.setString( "dumb");
            break;
        }
        case SnakeGroup::SMART:
        {
            text.setString( "smart");
            break;
        }
        default:
        {
            throw std::runtime_error{ "Unexpected snake group"};
        }
    }
    text.setPosition( { x + kSnakeStatsNameWidth + kSnakeStatsStatusWidth, y});
    window_.draw( text);

    text.setString( std::to_string( snake.points.size()));
    text.setPosition( { x + kSnakeStatsNameWidth + kSnakeStatsStatusWidth + kSnakeStatsGroupWidth, y});
    window_.draw( text);
}

} // ! namespace snake
