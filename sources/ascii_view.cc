#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <iomanip>

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
    Event            event;
    bool             case_sensitive;
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
    MenuEvent        event;
};

constexpr std::array<MenuKeyInfo, 7> kMenuKeysInfo{{
    { "\033[A", MenuEvent::KEY_PRESSED_ARROW_UP    },
    { "\033[B", MenuEvent::KEY_PRESSED_ARROW_DOWN  },
    { "\033[D", MenuEvent::KEY_PRESSED_ARROW_LEFT  },
    { "\033[C", MenuEvent::KEY_PRESSED_ARROW_RIGHT },
    {   "\x7f", MenuEvent::BACKSPACE               },
    {     "\n", MenuEvent::KEY_PRESSED_ENTER       },
    {     "\r", MenuEvent::KEY_PRESSED_ENTER       }
}};

constexpr Coordinate kStatusBarHeight  = 4;
constexpr Coordinate kGameFieldOffsetX = 1;
constexpr Coordinate kGameFieldOffsetY = 1;

//
// Colors constants
//

using colors::operator ""_c;

constexpr colors::Color kColorGameBox            = "#00ffe1"_c;
constexpr colors::Color kColorRabbit             = "#2f00ff"_c;
constexpr colors::Color kColorBone               = "#b1b1b1"_c;
constexpr colors::Color kMenuActiveColor         = "#bcffa4"_c;
constexpr colors::Color kMenuInactiveColor       = "#b6c9b7"_c;
constexpr colors::Color kMenuActiveStringColor   = "#ffffff"_c;
constexpr colors::Color kMenuInactiveStringColor = "#c8c8c8"_c;
constexpr colors::Color kInvalidColor            = "#676767"_c;

constexpr Coordinate kMenuElementHeight = 5;
constexpr float      kMenuOffsetY       = 0.10;
constexpr float      kMenuWidth         = 0.8;

constexpr Coordinate kSnakeStatusNameWidth   = 10;
constexpr Coordinate kSnakeStatusAliveWidth  = 7;
constexpr Coordinate kSnakeStatusLengthWidth = 3;
constexpr Coordinate kSnakeStatusGroupWidth  = 6;
constexpr Coordinate kSnakeStatusWidth       = kSnakeStatusNameWidth +
                                               kSnakeStatusAliveWidth +
                                               kSnakeStatusGroupWidth +
                                               kSnakeStatusLengthWidth;

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
    render_game_statistics( model);
}

void
AsciiView::RenderAll( const Model& model)
{
    set_color( "#ffffff"_c, false, "#000000"_c);
    clear_screen();
    draw_game_box();

    Render( model);

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
}

void
AsciiView::render_snake( const Snake& snake)
{
    auto it = snake.points.cbegin();

    set_color( snake.color);

    if ( snake.points.size() == 1 )
    {
        go_to_xy( it->x + kGameFieldOffsetX, it->y + kGameFieldOffsetY);
        std::cout << "○";
        ++it;
        return ;
    }

    for ( ; it != std::prev( snake.points.cend()); ++it )
    {
        write_game_symbol( *it, "○", snake.color);
    }

    write_game_symbol( *it, "●", snake.color);
}

void
AsciiView::render_rabbit( const Rabbit& rabbit)
{
    write_game_symbol( rabbit.point, "♥", kColorRabbit);
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
                      bool                 bold,
                      const colors::Color& background)
{
    std::cout << "\033["
              << (bold ? "1;" : "")
              << "38;2;"
              << static_cast<unsigned>(color.Red())   << ";"
              << static_cast<unsigned>(color.Green()) << ";"
              << static_cast<unsigned>(color.Blue())  << ";"
              << "48;2;"
              << static_cast<unsigned>(background.Red())   << ";"
              << static_cast<unsigned>(background.Green()) << ";"
              << static_cast<unsigned>(background.Blue())
              << "m";
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
AsciiView::render_bone( const Bone& bone)
{
    write_game_symbol( bone.point, "☠", kColorBone);
}

void
AsciiView::RenderMenu( const settings::Menu& settings)
{
    menu_render_ctx_t ctx{};

    ctx.offset_y = static_cast<Coordinate>( current_window_size_.second * kMenuOffsetY);

    set_color( "#ffffff"_c, false, "#000000"_c);
    clear_screen();

    std::size_t before_active = settings.GetNumberBeforeActive();

    while ( static_cast<Coordinate>( before_active) * kMenuElementHeight + ctx.offset_y >
            current_window_size_.second - kMenuElementHeight)
    {
        --ctx.offset_y;
    }

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
}

void
AsciiView::render_menu_button( const settings::MenuElement& menu_elem,
                               menu_render_ctx_t&           ctx)
{
    if ( (ctx.offset_y >= 0) &&
         (ctx.offset_y + kMenuElementHeight <= current_window_size_.second) )
    {
        if ( menu_elem.is_active )
        {
            set_color( kMenuActiveColor, true);
        } else
        {
            set_color( kMenuInactiveColor);
        }

        Coordinate width = static_cast<Coordinate>( current_window_size_.first * kMenuWidth);
        Coordinate x = (current_window_size_.first - width) / 2;
        Coordinate y = ctx.offset_y;
        Coordinate height = kMenuElementHeight - 1;
        Coordinate text_offset_x = x + width / 5;

        draw_box( x, y, width, height);

        go_to_xy( text_offset_x, y + height / 2);
        std::cout << menu_elem.name;
    }

    ctx.offset_y += kMenuElementHeight;
}

void
AsciiView::render_menu_snakes_list( const settings::MenuElement& menu_elem,
                                    menu_render_ctx_t&           ctx)
{
    const settings::SnakesList& snake_list = std::get<settings::SnakesList>( menu_elem.element);

    Coordinate width = static_cast<Coordinate>( current_window_size_.first * kMenuWidth);
    Coordinate x = (current_window_size_.first - width) / 2;
    Coordinate y = ctx.offset_y;
    Coordinate height = kMenuElementHeight - 1;
    Coordinate text_offset_x = x + width / 5;

    if ( (y >= 0) &&
         (y + kMenuElementHeight <= current_window_size_.second) )
    {
        bool is_active = (menu_elem.is_active) &&
                         (snake_list.active == snake_list.kNoActive);
        if ( is_active )
        {
            set_color( kMenuActiveColor, true);
        } else
        {
            set_color( kMenuInactiveColor);
        }

        draw_box( x, y, width, height);

        go_to_xy( text_offset_x, y + height / 2);
        std::cout << menu_elem.name;
    }

    y += kMenuElementHeight;
    width -= 2;
    x += 2;
    Coordinate text_offset_x_name  = x + 2;
    Coordinate text_offset_x_color = x + width * 3 / 4;

    for ( const settings::SnakeSetting& snake : snake_list.snakes )
    {
        if ( (y >= 0) &&
             (y + kMenuElementHeight <= current_window_size_.second) )
        {
            if ( snake.is_active )
            {
                set_color( kMenuActiveColor, true);
            } else
            {
                set_color( kMenuInactiveColor);
            }

            draw_box( x, y, width, height);

            if ( snake.active == settings::SnakeSetting::Active::NAME )
            {
                set_color( kMenuActiveStringColor, true);
            } else
            {
                set_color( kMenuInactiveStringColor);
            }
            go_to_xy( text_offset_x_name, y + height / 2);
            std::cout << "Snake name: " << snake.name;

            if ( snake.active == settings::SnakeSetting::Active::COLOR )
            {
                set_color( kMenuActiveStringColor, true);
            } else
            {
                set_color( kMenuInactiveStringColor);
            }
            go_to_xy( text_offset_x_color, y + height / 2);
            std::cout << "Snake color: ";

            if ( colors::IsValidColor( snake.color) )
            {
                colors::Color color{ snake.color};
                set_color( color);
            } else
            {
                set_color( kInvalidColor);
            }

            std::cout << snake.color;
        }
        y += kMenuElementHeight;
    }

    ctx.offset_y = y;
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
                menu_events_.push( key_info.event);
                DEBUG_INFO( "Got enum event: ", key_info.event);
            }
        }
        if ( !match )
        {
            if ( std::isprint( *buffer_pos) )
            {
                menu_events_.push( static_cast<MenuEvent>( *buffer_pos));
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

void
AsciiView::render_game_statistics( const Model& model)
{
    Statistics statistics = model.GetGameStatistics();

    Coordinate width  = current_window_size_.first;
    Coordinate height = current_window_size_.second;

    Coordinate x = 1;
    Coordinate y = height - kStatusBarHeight - 1;
    x = draw_group_stats( statistics.human, x, y, "Human Snakes");
    x = draw_group_stats( statistics.dumb,  x, y, "Dumb Snakes");
    x = draw_group_stats( statistics.smart, x, y, "Smart Snakes");

    Coordinate snake_status_x = x;

    set_color( kColorGameBox);

    while ( snake_status_x + kSnakeStatusWidth < width )
    {
        go_to_xy( snake_status_x + kSnakeStatusWidth + 1, y - 1);
        std::cout << "╤";
        go_to_xy( snake_status_x + kSnakeStatusWidth + 1, y + kStatusBarHeight);
        std::cout << "╧";
        draw_line( snake_status_x + kSnakeStatusWidth + 1, y,
                   snake_status_x + kSnakeStatusWidth + 1, y + kStatusBarHeight - 1,
                   "│");
        snake_status_x += kSnakeStatusWidth + 2;
    }

    snake_status_x = x;

    for ( const Snake& snake : model.GetSnakes() )
    {
        render_snake_status( snake, snake_status_x,  y, model.GetSnakeGroup( snake.id));
        snake_status_x += kSnakeStatusWidth + 2;
        if ( snake_status_x + kSnakeStatusWidth >= width )
        {
            snake_status_x = x;
            y += 1;
            if ( y > height - 1 )
            {
                break;
            }
        }
    }
}

Coordinate
AsciiView::draw_group_stats( const SnakeGroupStatistics& stats,
                             Coordinate                  x,
                             Coordinate                  y,
                             const std::string&          name)
{
    if ( stats.alive == 0 && stats.dead == 0 )
    {
        return x;
    }
    set_color( "#ffffff"_c, false, "#000000"_c);
    go_to_xy( x, y);
    std::cout << name;
    go_to_xy( x, y + 1);
    std::cout << std::setw( 16) << std::left << "Alive: "
              << std::setw( 4) << std::right << stats.alive;
    go_to_xy( x, y + 2);
    std::cout << std::setw( 16) << std::left << "Dead: "
              << std::setw( 4) << std::right << stats.dead;
    go_to_xy( x, y + 3);
    std::cout << std::setw( 16) << std::left << "Total length: "
              << std::setw( 4) << std::right << stats.total_length;

    set_color( kColorGameBox);
    go_to_xy( x + 20, y - 1);
    std::cout << "╤";
    go_to_xy( x + 20, y + kStatusBarHeight);
    std::cout << "╧";
    draw_line( x + 20, y, x + 20, y + kStatusBarHeight - 1, "│");

    return x + 21;
}

void
AsciiView::render_snake_status( const Snake& snake,
                                Coordinate   x,
                                Coordinate   y,
                                SnakeGroup   group)
{
    set_color( snake.color);

    go_to_xy( x, y);

    std::cout << std::left  << std::setw( kSnakeStatusNameWidth)   << snake.name
              << std::left  << std::setw( kSnakeStatusAliveWidth)  << (snake.is_alive ? "(alive)" : "(dead)");
    switch ( group )
    {
        case SnakeGroup::HUMAN:
        {
            std::cout << std::left << std::setw( kSnakeStatusGroupWidth) << " human";
            break;
        }
        case SnakeGroup::DUMB:
        {
            std::cout << std::left << std::setw( kSnakeStatusGroupWidth) << " dumb";
            break;
        }
        case SnakeGroup::SMART:
        {
            std::cout << std::left << std::setw( kSnakeStatusGroupWidth) << " smart";
            break;
        }
        default:
        {
            throw std::runtime_error{ "Unexpected snake group"};
        }
    }
    std::cout << std::right << std::setw( kSnakeStatusLengthWidth) << snake.points.size();
}

void
AsciiView::write_game_symbol( const Point&         point,
                              const char          *symbol,
                              const colors::Color& color) const
{
    go_to_xy( point.x + kGameFieldOffsetX, point.y + kGameFieldOffsetY);

    set_color( color, false, "#000000"_c);
    std::cout << symbol;
}

ViewUpdateCallbacks
AsciiView::GetCallbacks() const
{
    return ViewUpdateCallbacks{
        // Removing point
        [this]( const Point& point)
        {
            write_game_symbol( point, " ", "#ffffff"_c);
        },
        // Adding snake head
        [this]( const Snake& snake)
        {
            auto it = snake.points.rbegin();
            write_game_symbol( *it, "●", snake.color);
            ++it;
            write_game_symbol( *it, "○", snake.color);
        },
        // Adding rabbit
        [this]( const Point& point)
        {
            write_game_symbol( point, "♥", kColorRabbit);
        },
        // Adding bone
        [this]( const Point& point)
        {
            write_game_symbol( point, "☠", kColorBone);
        }
    };
}

void
AsciiView::Show()
{
    std::cout.flush();
}

} // ! namespace snake
