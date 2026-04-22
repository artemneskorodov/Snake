#ifndef SNAKE_GAME_COORDINATES_HH__
#define SNAKE_GAME_COORDINATES_HH__

#include <cstddef>
#include <memory>
#include <cassert>

namespace snake
{

///
/// @brief Game coordinates
///
using Coordinate = int;

//
// Direction utils
//

enum class Direction
{
    TOP    = 1,
    RIGHT  = 2,
    BOTTOM = 3,
    LEFT   = 4,
};

///
/// @brief Get degrees needed to rotate clockwise to get direction
///
inline int
DirectionToDegrees( Direction dir)
{
    return 90 * (static_cast<int>( dir) - 1);
}

inline bool
IsOpposite( Direction first,
            Direction second)
{
    int degrees_first  = DirectionToDegrees( first);
    int degrees_second = DirectionToDegrees( second);
    int difference = (360 + degrees_second - degrees_first) % 360;
    return (difference == 180);
}

//
// Point utils
//
struct Point
{
    constexpr
    Point() = default;

    constexpr
    Point( Coordinate x,
           Coordinate y)
     :  x{ x},
        y{ y}
    {
    }

    constexpr bool
    operator==( const Point& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    constexpr bool
    operator!=( const Point& rhs) const
    {
        return !((*this) == rhs);
    }

    constexpr Point
    operator+( const Point& rhs) const
    {
        return { x + rhs.x, y + rhs.y};
    }

    constexpr Point
    operator-( const Point& rhs) const
    {
        return { x - rhs.x, y - rhs.y};
    }

    Coordinate x{};
    Coordinate y{};

};

struct PointHash
{
    std::size_t
    operator()( const Point& point) const
    {
        return std::hash<int>()( point.x) ^ (std::hash<int>()( point.y) << 1);
    }
};

using Vector = Point;
using VectorHash = PointHash;

namespace game_coordinates_detail
{

constexpr std::array<Vector, 4> kVectorDirectionHashtab{{
    Vector{  0, -1},
    Vector{  1,  0},
    Vector{  0,  1},
    Vector{ -1,  0}
}};

} // ! namespace detail

inline Vector
DirectionToVector( Direction dir) noexcept
{
    assert( (static_cast<int>( dir) >= 1) &&
            (static_cast<int>( dir) <= 4));

    return game_coordinates_detail::kVectorDirectionHashtab[static_cast<int>( dir) - 1];
}

inline Direction
VectorToDirection( Vector vector) noexcept
{
    assert( (vector.x ==  1 && vector.y ==  0) ||
            (vector.x == -1 && vector.y ==  0) ||
            (vector.x ==  0 && vector.y ==  1) ||
            (vector.x ==  0 && vector.y == -1));

    for ( int i = 0; i != 4; ++i )
    {
        if ( game_coordinates_detail::kVectorDirectionHashtab[i] == vector )
        {
            return static_cast<Direction>( i + 1);
        }
    }

    return Direction{};
}

} // ! namespace snake

#endif // ! SNAKE_GAME_COORDINATES_HH__
