#ifndef SNAKE_MODEL_HH__
#define SNAKE_MODEL_HH__

#include <functional>
#include <vector>
#include <list>
#include <unordered_map>

#include "utils.hh"

namespace snake
{

using Coordinate = int;

using SnakeID = int;

using TickType = std::uint64_t;

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
is_opposite( Direction first,
             Direction second)
{
    return (first == Direction::TOP    && second == Direction::BOTTOM) ||
           (first == Direction::BOTTOM && second == Direction::TOP   ) ||
           (first == Direction::LEFT   && second == Direction::RIGHT ) ||
           (first == Direction::RIGHT  && second == Direction::LEFT  );
}

struct Point
{
    Point() = default;

    Point( Coordinate x,
           Coordinate y)
     :  x{ x},
        y{ y}
    {
    }

    bool
    operator==( const Point& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    bool
    operator!=( const Point& rhs) const
    {
        return !((*this) == rhs);
    }

    Point
    operator+( const Point& rhs) const
    {
        return { x + rhs.x, y + rhs.y};
    }

    Point
    operator-( const Point& rhs) const
    {
        return { x - rhs.x, y - rhs.y};
    }

    Coordinate x;
    Coordinate y;

};

struct PointHash
{
    std::size_t
    operator()( const Point& point) const
    {
        return std::hash<int>()( point.x) ^ (std::hash<int>()( point.y) << 1);
    }
};

class Model;
struct Snake;

using SnakeTicker = std::function<void( Model&, const Snake&)>;

struct Snake
{
    Snake( Coordinate  x,
           Coordinate  y,
           Direction   direction,
           SnakeID     id,
           SnakeTicker ticker)
     :  points    { {x, y}},
        direction { direction},
        id        { id},
        ticker    { std::move( ticker)}
    {
    }

    std::list<Point> points;
    Direction        direction;
    SnakeID          id;
    bool             is_alive{ true};
    SnakeTicker      ticker;

};

struct Bone
{
    Bone( Coordinate x,
          Coordinate y,
          TickType   death_tick)
     :  point      { x, y},
        death_tick { death_tick}
    {
    }

    Point    point;
    TickType death_tick;
    bool     is_alive{ true};

};

struct Rabbit
{
    Rabbit( Coordinate x,
            Coordinate y)
     :  point{ x, y}
    {
    }

    Point point;
    bool  is_alive{ true};
};

enum class CellType
{
    EMPTY,
    RABBIT,
    SNAKE,
    BONE,
};

class Model
{
public:
    Model( Coordinate width,
           Coordinate height)
     :  width_  { width},
        height_ { height}
    {
        set_cells_after_resize();
    }

    bool
    GameFinished() const
    {
        return game_finished_;
    }

    std::pair<Coordinate, Coordinate>
    GetFieldSize() const
    {
        return { width_, height_};
    }

    void
    SetFieldSize( Coordinate width,
                  Coordinate height)
    {
        width_ = width;
        height_ = height;
        set_cells_after_resize();
    }

    void Tick();

    //
    // Snake control methods
    //

    SnakeID AddSnake( SnakeTicker ticker = []( Model&, const Snake&) {});

    void
    SetSnakeDirection( SnakeID   id,
                       Direction direction)
    {
        if ( (snakes_[id].points.size() != 1) &&
             is_opposite( direction, snakes_[id].direction) )
        {
            return ;
        }
        snakes_[id].direction = direction;
    }

    const Snake&
    GetSnake( SnakeID id) const &
    {
        return snakes_[id];
    }

    const std::vector<Snake>&
    GetSnakes() const &
    {
        return snakes_;
    }

    //
    // Rabbits control methods
    //

    const std::vector<Rabbit>&
    GetRabbits() const &
    {
        return rabbits_;
    }

    CellType
    GetCellType( const Point& point) const
    {
        return cells_.at( point);
    }

    CellType
    GetCellType( Coordinate x, Coordinate y) const
    {
        return cells_.at( { x, y});
    }

    const std::vector<Bone>&
    GetBones() const &
    {
        return bones_;
    }

private:
    void add_rabbit();
    void remove_snake( Snake& snake);
    void set_cells_after_resize();
    void add_bone( const Point& point, TickType lifetime);

private:
    void tick_snake_positions_update();
    void tick_snake_rabbit_collisions_check();
    void tick_snake_snake_collisions_check();
    void tick_check_rabbits();
    void tick_check_bones_lifetime();
    void tick_snake_bone_collisions_check();

private:
    static constexpr int kRabbitsSpawnRateAvg   = 10;
    static constexpr int kRabbitsSpawnRateSigma = 5;
    static constexpr double kBoneSpawnProbability = 0.7;
    static constexpr TickType kBonesLifetimeAvg = 50;
    static constexpr TickType kBonesLifetimeSigma = 10;

private:
    Coordinate          width_;
    Coordinate          height_;
    bool                game_finished_       { false};
    std::vector<Snake>  snakes_              {};
    std::vector<Rabbit> rabbits_             {};
    std::vector<Bone>   bones_               {};
    std::size_t         snakes_number_       { 0};
    int                 rabbits_counter_     { 0};
    int                 next_rabbit_counter_ { utils::random_normal( kRabbitsSpawnRateAvg,
                                                                     kRabbitsSpawnRateSigma)};
    TickType            tick_{ 0};
    std::unordered_map<Point, CellType, PointHash> cells_;

};

} // ! namespace snake

#endif // ! SNAKE_MODEL_HH__
