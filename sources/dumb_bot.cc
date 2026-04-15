#include <cstdlib>

#include "model.hh"
#include "bots.hh"

namespace snake
{
namespace bots
{

namespace
{

template<typename T>
T
random_of( std::initializer_list<T> list)
{
    std::size_t size = list.size();
    auto it = list.begin();
    std::size_t result = std::rand() % size;
    for ( std::size_t i = 0; i != result; ++i )
    {
        ++it;
    }
    return *it;
}

} // ! anonymous namespace

void
TickDumbBot( Model&       model,
             const Snake& snake)
{
    const Point& head = snake.points.back();

    double minimal_distance_sq = -1.0;
    Coordinate minimal_distance_x = -1;
    Coordinate minimal_distance_y = -1;
    for ( const Rabbit& rabbit : model.GetRabbits() )
    {
        if ( !rabbit.is_alive )
        {
            continue;
        }

        double dx = static_cast<double>( rabbit.point.x - head.x);
        double dy = static_cast<double>( rabbit.point.y - head.y);

        double dr_sq = dx * dx + dy * dy;

        if ( (dr_sq < minimal_distance_sq) ||
             (minimal_distance_sq < 0) )
        {
            minimal_distance_sq = dr_sq;
            minimal_distance_x = rabbit.point.x;
            minimal_distance_y = rabbit.point.y;
        }
    }

    if ( head.x < minimal_distance_x )
    {
        if ( snake.direction == Direction::LEFT )
        {
            model.SetSnakeDirection( snake.id,
                                     random_of( {Direction::BOTTOM, Direction::TOP}));
        } else
        {
            model.SetSnakeDirection( snake.id, Direction::RIGHT);
        }
    } else if ( head.x > minimal_distance_x )
    {
        if ( snake.direction == Direction::RIGHT )
        {
            model.SetSnakeDirection( snake.id,
                                     random_of( {Direction::BOTTOM, Direction::TOP}));
        } else
        {
            model.SetSnakeDirection( snake.id, Direction::LEFT);
        }
    } else if ( head.y < minimal_distance_y )
    {
        if ( snake.direction == Direction::TOP )
        {
            model.SetSnakeDirection( snake.id,
                                     random_of( {Direction::LEFT, Direction::RIGHT}));
        } else
        {
            model.SetSnakeDirection( snake.id, Direction::BOTTOM);
        }
    } else if ( head.y > minimal_distance_y )
    {
        if ( snake.direction == Direction::BOTTOM )
        {
            model.SetSnakeDirection( snake.id,
                                     random_of( {Direction::LEFT, Direction::RIGHT}));
        } else
        {
            model.SetSnakeDirection( snake.id, Direction::TOP);
        }
    }
}

} // ! namespace bots
} // ! namespace snake
