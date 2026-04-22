#ifndef SNAKE_COLORS_HH__
#define SNAKE_COLORS_HH__

#include <string>
#include <cassert>
#include <cstdint>

#include <SFML/Graphics.hpp>

namespace snake
{
namespace colors
{

class Color
{
public:
    constexpr
    Color( std::string_view color)
     :  red   { static_cast<uint8_t>( str_to_int( color[1]) * 0x10 + str_to_int( color[2]))},
        green { static_cast<uint8_t>( str_to_int( color[3]) * 0x10 + str_to_int( color[4]))},
        blue  { static_cast<uint8_t>( str_to_int( color[5]) * 0x10 + str_to_int( color[6]))}
    {
    }

    constexpr
    operator sf::Color() const
    {
        return sf::Color{ red, green, blue};
    }

    constexpr std::uint8_t Red()   const { return red; }
    constexpr std::uint8_t Green() const { return green; }
    constexpr std::uint8_t Blue()  const { return blue; }

private:
    constexpr std::uint8_t
    str_to_int( char c)
    {
        if ( c >= '0' && c <= '9' )
        {
            return c - '0';
        }
        if ( c >= 'a' && c <= 'f' )
        {
            return c - 'a' + 0xA;
        }
        if ( c >= 'A' && c <= 'F' )
        {
            return c - 'A' + 0xA;
        }
        return 0;
    }

private:
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;

};

inline bool
IsValidColor( std::string str)
{
    if ( str.length() != 6 + 1 )
    {
        return false;
    }

    if ( str[0] != '#' )
    {
        return false;
    }

    for ( std::size_t i = 1; i != 6 + 1; ++i )
    {
        if ( str[i] >= '0' && str[i] <= '9' )
        {
            continue;
        }
        if ( str[i] >= 'a' && str[i] <= 'f' )
        {
            continue;
        }
        if ( str[i] >= 'A' && str[i] <= 'F' )
        {
            continue;
        }
        return false;
    }
    return true;
}

constexpr inline Color
operator ""_c( const char *string, size_t size)
{
    return Color{ std::string_view{ string, size}};
}

} // ! namespace colors
} // ! namespace snake

#endif // ! SNAKE_VIEW_COLORS_HH__
