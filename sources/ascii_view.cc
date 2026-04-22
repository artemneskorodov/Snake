#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "view.hh"
#include "ascii_view.hh"
#include "colors.hh"

#include "logging.hh"

namespace snake
{

namespace
{

struct KeyInfo
{
    std::string_view key;
    Event event;
    bool case_sensitive;
};

constexpr std::array<KeyInfo, 9> kKeysInfo{{
    {      "w", Event{ 0, Event::KEY_PRESSED_PLAYER_TOP   }, false},
    {      "s", Event{ 0, Event::KEY_PRESSED_PLAYER_BOTTOM}, false},
    {      "a", Event{ 0, Event::KEY_PRESSED_PLAYER_LEFT  }, false},
    {      "d", Event{ 0, Event::KEY_PRESSED_PLAYER_RIGHT }, false},
    { "\033[A", Event{ 1, Event::KEY_PRESSED_PLAYER_TOP   },  true},
    { "\033[B", Event{ 1, Event::KEY_PRESSED_PLAYER_BOTTOM},  true},
    { "\033[D", Event{ 1, Event::KEY_PRESSED_PLAYER_LEFT  },  true},
    { "\033[C", Event{ 1, Event::KEY_PRESSED_PLAYER_RIGHT },  true},
    {      "q", Event::KEY_PRESSED_EXIT,                     false},
}};

struct MenuKeyInfo
{
    std::string_view key;
    MenuEvent event;
};

constexpr std::array<MenuKeyInfo, 7> kMenuKeysInfo{{
    { "\033[A", MenuEvent::KEY_PRESSED_ARROW_UP    },
    { "\033[B", MenuEvent::KEY_PRESSED_ARROW_DOWN  },
    { "\033[D", MenuEvent::KEY_PRESSED_ARROW_LEFT  },
    { "\033[C", MenuEvent::KEY_PRESSED_ARROW_RIGHT },
    {     "\b", MenuEvent::BACKSPACE               },
    {     "\n", MenuEvent::KEY_PRESSED_ENTER       },
    {     "\r", MenuEvent::KEY_PRESSED_ENTER       }
}};

constexpr Coordinate kStatusBarHeight = 4;
constexpr Coordinate kGameFieldOffsetX = 1;
constexpr Coordinate kGameFieldOffsetY = 1;

//
// Colors constants
//

using colors::operator ""_c;

constexpr colors::Color kColorGameBox = "#00ffe1"_c;
constexpr colors::Color kColorRabbit = "#2f00ff"_c;
constexpr colors::Color kColorBone = "#b1b1b1"_c;

constexpr std::array<colors::Color, 9> kColorSnake{{
    "#bf00ff"_c,
    "#26ff00"_c,
    "#d4f903"_c,
    "#ff5b02"_c,
    "#00ffd5"_c,
    "#0091ff"_c,
    "#8400ff"_c,
    "#ff00d9"_c,
    "#ff0000"_c,
}};


constexpr inline colors::Color
snake_color( SnakeID id)
{
    return kColorSnake[id % kColorSnake.size()];
}

} // ! anonymous namespace

AsciiView::AsciiView()
{
    termios attr;
    tcgetattr( STDIN_FILENO, &attr);
    console_attr_saved_ = attr;
    cfmakeraw( &attr);
    tcsetattr( STDIN_FILENO, 0, &attr);

    current_window_size_ = get_window_size();
}

AsciiView::~AsciiView()
{
    tcsetattr( STDIN_FILENO, 0, &console_attr_saved_);
}

void
AsciiView::Render( const Model& model)
{
    clear_screen();

    draw_game_box();

    Coordinate status_offset = 1;
    Coordinate snakes_number = model.GetSnakes().size();
    Coordinate width = current_window_size_.first;
    Coordinate pixels_per_snake = width / snakes_number;

    for ( const Snake& snake : model.GetSnakes() )
    {
        render_snake_status( snake, status_offset);
        status_offset += pixels_per_snake;
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

    std::cout.flush();
}

void
AsciiView::render_snake( const Snake& snake)
{
    auto it = snake.points.cbegin();

    set_color( snake_color( snake.id));

    if ( snake.points.size() == 1 )
    {
        go_to_xy( it->x + kGameFieldOffsetX, it->y + kGameFieldOffsetY);
        std::cout << "○";
        ++it;
        return ;
    }

    for ( ; it != std::prev( snake.points.cend()); ++it )
    {
        go_to_xy( it->x + kGameFieldOffsetX, it->y + kGameFieldOffsetY);
        std::cout << "○";
    }

    go_to_xy( it->x + kGameFieldOffsetX, it->y + kGameFieldOffsetY);
    std::cout << "●";
}

void
AsciiView::render_rabbit( const Rabbit& rabbit)
{
    go_to_xy( rabbit.point.x + kGameFieldOffsetX, rabbit.point.y + kGameFieldOffsetY);
    set_color( kColorRabbit);
    std::cout << "♥";
}

std::pair<Coordinate, Coordinate>
AsciiView::GetGameFieldSize() const
{
    return {current_window_size_.first - (kGameFieldOffsetX + 1),
            current_window_size_.second - (kGameFieldOffsetY + 1 + 1 + kStatusBarHeight)};
}

std::pair<Coordinate, Coordinate>
AsciiView::get_window_size()
{
    winsize winsz;
    ioctl( STDIN_FILENO, TIOCGWINSZ, &winsz);
    return { winsz.ws_col, winsz.ws_row};
}

void
AsciiView::UpdateEvents()
{
    fd_set read_fds;
    FD_ZERO( &read_fds);
    FD_SET( STDIN_FILENO, &read_fds);

    timeval timeout{};

    if ( select( 1, &read_fds, nullptr, nullptr, &timeout) == 0 )
    {
        return ;
    }

    char buffer[256];
    ssize_t sz = read( STDIN_FILENO, buffer, 256);
    if ( sz < 0 )
    {
        throw std::runtime_error( "Error while reading stdin: " +
                                  std::string{ std::strerror( errno)});
    } else if ( sz == 0 )
    {
        throw std::runtime_error( "Closed stdin");
    }

    for ( ssize_t pos = 0; pos != sz; )
    {
        ssize_t max_length = sz - pos;
        bool match = false;
        const char *buffer_pos = &buffer[pos];

        for ( const KeyInfo& key_info : kKeysInfo )
        {
            ssize_t length  = key_info.key.length();
            const char *str = key_info.key.data();

            if ( length > max_length )
            {
                continue;
            }

            if ( (key_info.case_sensitive  && strncmp( str, buffer_pos, length) == 0) ||
                 (!key_info.case_sensitive && strncasecmp( str, buffer_pos, length) == 0) )
            {
                DEBUG_INFO( "Got event: ", key_info.key);
                match = true;
                pos += length;
                events_.push( key_info.event);
            }
        }
        if ( !match )
        {
            ++pos;
        }
    }

    auto size = get_window_size();
    if ( size != current_window_size_ )
    {
        current_window_size_ = size;
        events_.push( Event::WINDOW_SIZE_CHANGED);
    }
}

void
AsciiView::clear_screen()
{
    std::cout << "\033[H\033[J";
}

void
AsciiView::go_to_xy( Coordinate x,
                     Coordinate y)
{
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
}

void
AsciiView::set_color( const colors::Color& color,
                      bool                 bold)
{
    std::cout << "\033[" << (bold ? "1;" : "") << "38;2;"
              << static_cast<unsigned>( color.Red())   << ";"
              << static_cast<unsigned>( color.Green()) << ";"
              << static_cast<unsigned>( color.Blue())  << "m";
}

void
AsciiView::draw_game_box()
{
    set_color( kColorGameBox);

    Coordinate width  = current_window_size_.first  - 1;
    Coordinate height = current_window_size_.second - 1;

    go_to_xy(0, 0);
    std::cout << "╔";
    go_to_xy( 0, height);
    std::cout << "╚";
    go_to_xy( width, height);
    std::cout << "╝";
    go_to_xy( width, 0);
    std::cout << "╗";
    go_to_xy( 0, height - 1 - kStatusBarHeight);
    std::cout << "╠";
    go_to_xy( width, height - 1 - kStatusBarHeight);
    std::cout << "╣";

    draw_line( 1, 0, width - 1, 0, "═");
    draw_line( 1, height, width - 1, height, "═");
    draw_line( 1, height - 1 - kStatusBarHeight, width - 1, height - 1 - kStatusBarHeight, "═");

    draw_line( 0, 1, 0, height - kStatusBarHeight - 2, "║");
    draw_line( width, 1, width, height - kStatusBarHeight - 2, "║");

    draw_line( 0, height - kStatusBarHeight, 0, height - 1, "║");
    draw_line( width, height - kStatusBarHeight, width, height - 1, "║");
}

void
AsciiView::draw_line( Coordinate  x1,
                      Coordinate  y1,
                      Coordinate  x2,
                      Coordinate  y2,
                      const char *symbol)
{
    Coordinate dx = std::abs( x2 - x1);
    Coordinate dy = std::abs( y2 - y1);

    Coordinate dir_x = (x2 > x1) ? 1 : -1;
    Coordinate dir_y = (y2 > y1) ? 1 : -1;

    Coordinate error = dx - dy;

    for ( ; ; )
    {
        go_to_xy( x1, y1);
        std::cout << symbol;

        if ( (x1 == x2) && (y1 == y2) )
        {
            break;
        }

        Coordinate double_error = 2 * error;
        if ( double_error > -dy )
        {
            error -= dy;
            x1 += dir_x;
        }
        if ( double_error < dx )
        {
            error += dx;
            y1 += dir_y;
        }
    }
}

void
AsciiView::render_snake_status( const Snake& snake,
                                Coordinate status_offset)
{
    set_color( snake_color( snake.id));
    Coordinate height = current_window_size_.second;
    go_to_xy( status_offset, height - kStatusBarHeight);
    std::cout << "Snake" << snake.id;
    go_to_xy( status_offset, height - kStatusBarHeight + 1);
    std::cout << (snake.is_alive ? "alive" : "dead");
}

void
AsciiView::render_bone( const Bone& bone)
{
    go_to_xy( bone.point.x + kGameFieldOffsetX, bone.point.y + kGameFieldOffsetY);
    set_color( kColorBone);
    std::cout << "☠";
}

void
AsciiView::RenderMenu( const settings::Menu& settings)
{
    menu_render_ctx_t ctx{};

    clear_screen();

    for ( const settings::MenuElement& menu_element : settings.GetMenu() )
    {
        if ( std::holds_alternative<settings::Button>( menu_element.element) )
        {
            render_menu_button( menu_element, ctx);
        } else if ( std::holds_alternative<settings::SnakesList>( menu_element.element) )
        {
            render_menu_snakes_list( menu_element, ctx);
        }
    }

    std::cout.flush();
}

void
AsciiView::render_menu_button( const settings::MenuElement& menu_elem,
                               menu_render_ctx_t&           ctx)
{
    draw_box( 5, ctx.offset_y, 50, 4);

    go_to_xy( 5 + 2, ctx.offset_y + 2);
    std::cout << menu_elem.name;

    ctx.offset_y += 5;
}

void
AsciiView::render_menu_snakes_list( const settings::MenuElement& menu_elem,
                                    menu_render_ctx_t&           ctx)
{
    draw_box( 5, ctx.offset_y, 50, 4);

    go_to_xy( 5 + 2, ctx.offset_y + 2);
    std::cout << menu_elem.name;

    ctx.offset_y += 5;

    const settings::SnakesList& snake_list = std::get<settings::SnakesList>( menu_elem.element);
    for ( const settings::SnakeSetting& snake : snake_list.snakes )
    {
        draw_box( 5 + 2, ctx.offset_y, 50 - 2, 4);

        go_to_xy( 5 + 2 + 2, ctx.offset_y + 2);
        std::cout << snake.name << snake.color;
        ctx.offset_y += 5;
    }
}

void
AsciiView::UpdateMenuEvents()
{
    fd_set read_fds;
    FD_ZERO( &read_fds);
    FD_SET( STDIN_FILENO, &read_fds);

    timeval timeout{};

    if ( select( 1, &read_fds, nullptr, nullptr, &timeout) == 0 )
    {
        return ;
    }

    char buffer[256];
    ssize_t sz = read( STDIN_FILENO, buffer, 256);
    if ( sz < 0 )
    {
        throw std::runtime_error( "Error while reading stdin: " +
                                  std::string{ std::strerror( errno)});
    } else if ( sz == 0 )
    {
        throw std::runtime_error( "Closed stdin");
    }

    for ( ssize_t pos = 0; pos != sz; )
    {
        ssize_t max_length = sz - pos;
        bool match = false;
        const char *buffer_pos = &buffer[pos];

        for ( const MenuKeyInfo& key_info : kMenuKeysInfo )
        {
            ssize_t length  = key_info.key.length();
            const char *str = key_info.key.data();

            if ( length > max_length )
            {
                continue;
            }

            if ( strncmp( str, buffer_pos, length) == 0 )
            {
                match = true;
                pos += length;
                menu_events_.emplace_back( key_info.event);
                DEBUG_INFO( "Got enum event: ", key_info.event);
            }
        }
        if ( !match )
        {
            if ( std::isprint( *buffer_pos) )
            {
                menu_events_.emplace_back( static_cast<MenuEvent>( *buffer_pos));
                DEBUG_INFO( "Got char event: ", *buffer_pos);
            } else
            {
                DEBUG_INFO( "No event symbol: ", static_cast<int>( *buffer_pos));
            }
            ++pos;
        }
    }

    auto size = get_window_size();
    if ( size != current_window_size_ )
    {
        current_window_size_ = size;
    }
}

void
AsciiView::draw_box( Coordinate x,
                     Coordinate y,
                     Coordinate width,
                     Coordinate height)
{
    go_to_xy( x, y);
    std::cout << "┌";
    go_to_xy( x + width, y);
    std::cout << "┐";
    go_to_xy( x, y + height);
    std::cout << "└";
    go_to_xy( x + width, y + height);
    std::cout << "┘";

    draw_line( x + 1, y, x + width - 1, y, "─");
    draw_line( x + 1, y + height, x + width - 1, y + height, "─");
    draw_line( x, y + 1, x, y + height - 1, "│");
    draw_line( x + width, y + 1, x + width, y + height - 1, "│");
}

} // ! namespace snake
