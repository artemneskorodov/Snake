#ifndef SNAKE_MODEL_HH__
#define SNAKE_MODEL_HH__

#include <functional>
#include <vector>
#include <list>
#include <unordered_map>

#include "utils.hh"
#include "colors.hh"

namespace snake
{

using Coordinate = int;

using SnakeID = int;

using TickType = std::int64_t;

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

inline Point
DirectionToVector( Direction dir)
{
    assert( (static_cast<int>( dir) >= 1) &&
            (static_cast<int>( dir) <= 4));
    std::array<Point, 4> hashtab{{
        Point{  0, -1},
        Point{  1,  0},
        Point{  0,  1},
        Point{ -1,  0}
    }};
    return hashtab[static_cast<int>( dir) - 1];
}

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
    Snake( SnakeID       id,
           SnakeTicker   ticker,
           std::string   name,
           colors::Color color)
     :  id        { id},
        ticker    { std::move( ticker)},
        name      { std::move( name)},
        color     { color}
    {
    }

    std::list<Point> points    {};
    Direction        direction {};
    bool             is_alive  { true};
    std::size_t      killed    { 0};
    SnakeID          id;
    SnakeTicker      ticker;
    std::string      name;
    colors::Color    color;

    static constexpr int kScoresPerLength = 1;
    static constexpr int kScoresPerKill   = 3;

    int
    GetScores() const
    {
        return kScoresPerLength * points.size() +
               kScoresPerKill   * killed;
    }

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

struct SnakeGroupStatistics
{
    std::size_t dead;
    std::size_t alive;
    std::size_t total_length;
};

enum class SnakeGroup
{
    HUMAN,
    DUMB,
    SMART,
};

struct Statistics
{
    SnakeGroupStatistics human;
    SnakeGroupStatistics dumb;
    SnakeGroupStatistics smart;

};

struct ViewUpdateCallbacks
{
    std::function<void( const Point&)> removed_point_cb{};
    std::function<void( const Snake&)> snake_head_push_cb{};
    std::function<void( const Point&)> rabbit_add_cb{};
    std::function<void( const Point&)> bone_add_cb{};

};

class Model
{
public:
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

    Statistics
    GetGameStatistics() const
    {
        Statistics statistics{};

        auto collect_group_statistics = [this]( SnakeGroupStatistics& stats, const std::vector<SnakeID>& group)
        {
            for ( SnakeID id : group )
            {
                const Snake& snake = GetSnake( id);
                if ( snake.is_alive )
                {
                    ++stats.alive;
                    stats.total_length += snake.points.size();
                } else
                {
                    ++stats.dead;
                }
            }
        };
        collect_group_statistics( statistics.human, humans_snakes_group_);
        collect_group_statistics( statistics.dumb,  dumb_snakes_group_);
        collect_group_statistics( statistics.smart, smart_snakes_group_);

        return statistics;
    }

    SnakeGroup
    GetSnakeGroup( SnakeID id) const
    {
        if ( std::find( humans_snakes_group_.begin(),
                        humans_snakes_group_.end(),
                        id) != humans_snakes_group_.end() )
        {
            return SnakeGroup::HUMAN;
        }
        if ( std::find( dumb_snakes_group_.begin(),
                        dumb_snakes_group_.end(),
                        id) != dumb_snakes_group_.end() )
        {
            return SnakeGroup::DUMB;
        }
        if ( std::find( smart_snakes_group_.begin(),
                        smart_snakes_group_.end(),
                        id) != smart_snakes_group_.end() )
        {
            return SnakeGroup::SMART;
        }
        return SnakeGroup{};
    }

    //
    // Snake control methods
    //

    SnakeID AddSnake( std::string   name,
                      colors::Color color,
                      SnakeGroup    group,
                      SnakeTicker   ticker = []( Model&, const Snake&) {});

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

    void
    SetCallBacks( const ViewUpdateCallbacks& callbacks)
    {
        view_update_callbacks_ = callbacks;
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
    static constexpr int      kRabbitsSpawnRateAvg   = 25;
    static constexpr int      kRabbitsSpawnRateSigma = 5;
    static constexpr double   kBoneSpawnProbability  = 0.7;
    static constexpr TickType kBonesLifetimeAvg      = 100;
    static constexpr TickType kBonesLifetimeSigma    = 20;
    static constexpr float    kMinSnakeOffset        = 0.2;

private:
    Coordinate          width_               { 0};
    Coordinate          height_              { 0};
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
    std::vector<SnakeID> humans_snakes_group_{};
    std::vector<SnakeID> dumb_snakes_group_{};
    std::vector<SnakeID> smart_snakes_group_{};

    ViewUpdateCallbacks  view_update_callbacks_{};

};

} // ! namespace snake

#endif // ! SNAKE_MODEL_HH__
