#include "model.hh"
#include "bots.hh"

namespace snake
{
namespace bots
{

void
TickDumbBot( const Model& model,
             Snake& snake)
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
        snake.direction = Direction::RIGHT;
    } else if ( head.x > minimal_distance_x )
    {
        snake.direction = Direction::LEFT;
    } else if ( head.y < minimal_distance_y )
    {
        snake.direction = Direction::BOTTOM;
    } else if ( head.y > minimal_distance_y )
    {
        snake.direction = Direction::TOP;
    }
}

} // ! namespace bots
} // ! namespace snake
