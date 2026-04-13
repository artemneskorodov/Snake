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

struct Point
{
    Point( Coordinate x,
           Coordinate y)
     :  x{ x},
        y{ y}
    {
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

    SnakeID
    AddSnake( Coordinate x,
              Coordinate y,
              Direction  direction)
    {
        snakes_.emplace_back( x, y, direction);
        return static_cast<SnakeID>( snakes_.size() - 1);
    }

    void
    SetSnakeDirection( SnakeID   id,
                       Direction direction)
    {
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

private:
    Coordinate         width_;
    Coordinate         height_;
    bool               game_finished_ { false};
    std::vector<Snake> snakes_        {};

};

} // ! namespace snake

#endif // ! SNAKE_MODEL_HH__
