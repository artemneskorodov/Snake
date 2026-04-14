#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "view.hh"
#include "ascii_view.hh"

#include "logging.hh"

namespace snake
{

namespace
{

constexpr uint32_t kSnakeColor = 0x17ab55;

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

} // ! anonymous namespace

AsciiView::AsciiView()
{
    termios attr;
    tcgetattr( STDIN_FILENO, &attr);
    console_attr_saved_ = attr;
    cfmakeraw( &attr);
    tcsetattr( STDIN_FILENO, 0, &attr);
}

AsciiView::~AsciiView()
{
    tcsetattr( STDIN_FILENO, 0, &console_attr_saved_);
}

void
AsciiView::Render( const Model& model)
{
    clear_screen();
    std::pair<Coordinate, Coordinate> winsz = model.GetFieldSize();

    Coordinate width  = winsz.first  - 1;
    Coordinate height = winsz.second - 1;

    draw_box( 0, 0, width, height,
              "╔", "║", "╚", "═",
              "╝", "║", "╗", "═");

    for ( const Snake& snake : model.GetSnakes() )
    {
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

    std::cout.flush();
}

void
AsciiView::render_snake( const Snake& snake)
{
    auto it = snake.points.cbegin();

    set_color( kSnakeColor);

    go_to_xy( it->x, it->y);
    std::cout << "○";
    ++it;

    // If body is 1 element length
    if ( it == snake.points.cend() )
    {
        return ;
    }

    for ( ; it != std::prev( snake.points.cend()); ++it )
    {
        go_to_xy( it->x, it->y);
        std::cout << "⩔";
    }

    go_to_xy( it->x, it->y);
    std::cout << "●";
    set_color( 0x000000);
}

void
AsciiView::render_rabbit( const Rabbit& rabbit)
{
    go_to_xy( rabbit.point.x, rabbit.point.y);
    set_color( 0x0a15eb);
    std::cout << "♥";
    set_color( 0x000000);
}

std::pair<Coordinate, Coordinate>
AsciiView::GetWindowSize() const
{
    return current_window_size_;
}

void
AsciiView::RequestWindowSize()
{
    winsize winsz;
    ioctl( STDIN_FILENO, TIOCGWINSZ, &winsz);
    current_window_size_ = { winsz.ws_col, winsz.ws_row};
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
                events_.emplace_back( key_info.event);
            }
        }
        if ( !match )
        {
            ++pos;
        }
    }

    update_common_events();
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
AsciiView::set_color( uint32_t rgb)
{
    int r = (rgb & 0xff0000) >> (8 * 2);
    int g = (rgb & 0x00ff00) >> (8 * 1);
    int b = (rgb & 0x0000ff) >> (8 * 0);
    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
}

void
AsciiView::draw_box( Coordinate  x,
                     Coordinate  y,
                     Coordinate  width,
                     Coordinate  height,
                     const char *top_left,
                     const char *left,
                     const char *bottom_left,
                     const char *bottom,
                     const char *bottom_right,
                     const char *right,
                     const char *top_right,
                     const char *top)
{
    set_color( 0xffffff);

    go_to_xy( x, y);
    std::cout << top_left;

    go_to_xy( x + width, y);
    std::cout << top_right;

    go_to_xy( x, y + height);
    std::cout << bottom_left;

    go_to_xy( x + width, y + height);
    std::cout << bottom_right;

    draw_line( x + 1,     y,          x + width - 1, y,              top);
    draw_line( x,         y + 1,      x,             y + height - 1, left);
    draw_line( x + 1,     y + height, x + width - 1, y + height,     bottom);
    draw_line( x + width, y + 1,      x + width,     y + height - 1, right);
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

} // ! namespace snake
