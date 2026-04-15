#ifndef SNAKE_MODEL_HH__
#define SNAKE_MODEL_HH__

#include <functional>
#include <vector>
#include <list>

namespace snake
{

using Coordinate = int;

using SnakeID = int;

enum class Direction
{
    TOP,
    LEFT,
    BOTTOM,
    RIGHT,
};

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

    Coordinate x;
    Coordinate y;

};

struct Snake
{
    Snake( Coordinate x,
           Coordinate y,
           Direction direction)
     :  points    { {x, y}},
        direction { direction}
    {
    }

    std::list<Point> points;
    Direction        direction;
    bool             is_alive{ true};
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

class Model
{
public:
    Model( Coordinate width,
           Coordinate height)
     :  width_  { width},
        height_ { height}
    {
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
    }

    void Tick();

    //
    // Snake control methods
    //

    SnakeID
    AddSnake( Coordinate x,
              Coordinate y,
              Direction  direction)
    {
        snakes_.emplace_back( x, y, direction);
        ++snakes_number_;
        return static_cast<SnakeID>( snakes_.size() - 1);
    }

    void
    SetSnakeDirection( SnakeID   id,
                       Direction direction)
    {
        if ( is_opposite( direction, snakes_[id].direction) )
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

    void
    AddRabbit( Coordinate x,
               Coordinate y)
    {
        rabbits_.emplace_back( x, y);
        ++rabbits_number_;
    }

    std::size_t
    GetRabbitsNumber() const
    {
        return rabbits_number_;
    }

    const std::vector<Rabbit>&
    GetRabbits() const &
    {
        return rabbits_;
    }

private:
    Coordinate          width_;
    Coordinate          height_;
    bool                game_finished_  { false};
    std::vector<Snake>  snakes_         {};
    std::vector<Rabbit> rabbits_        {};
    std::size_t         rabbits_number_ { 0};
    std::size_t         snakes_number_  { 0};

};

} // ! namespace snake

#endif // ! SNAKE_MODEL_HH__
