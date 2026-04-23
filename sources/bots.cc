#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "model.hh"
#include "bots.hh"

namespace snake
{
namespace bots
{

namespace
{

Point
get_closest_target( const Model& model,
                    const Snake& snake)
{
    const Point& head = snake.points.back();

    Coordinate minimal_distance_x = -1;
    Coordinate minimal_distance_y = -1;

    Coordinate minimal_distance = -1;

    for ( const Rabbit& rabbit : model.GetRabbits() )
    {
        if ( !rabbit.is_alive )
        {
            continue;
        }

        Coordinate dx = rabbit.point.x - head.x;
        Coordinate dy = rabbit.point.y - head.y;

        dx = (dx > 0) ? dx : -dx;
        dy = (dy > 0) ? dy : -dy;

        Coordinate distance = dx + dy;

        if ( (distance < minimal_distance) ||
             (minimal_distance < 0) )
        {
            minimal_distance = distance;
            minimal_distance_x = rabbit.point.x;
            minimal_distance_y = rabbit.point.y;
        }
    }

    return { minimal_distance_x, minimal_distance_y};
}

Direction
get_direction( const Point& from,
               const Point& to,
               Direction    direction)
{
    if ( from.x < to.x )
    {
        if ( direction == Direction::LEFT )
        {
            return utils::random_of( {Direction::BOTTOM, Direction::TOP});
        } else
        {
            return Direction::RIGHT;
        }
    } else if ( from.x > to.x )
    {
        if ( direction == Direction::RIGHT )
        {
            return utils::random_of( {Direction::BOTTOM, Direction::TOP});
        } else
        {
            return Direction::LEFT;
        }
    } else if ( from.y < to.y )
    {
        if ( direction == Direction::TOP )
        {
            return utils::random_of( {Direction::LEFT, Direction::RIGHT});
        } else
        {
            return Direction::BOTTOM;
        }
    } else if ( from.y > to.y )
    {
        if ( direction == Direction::BOTTOM )
        {
            return utils::random_of( {Direction::LEFT, Direction::RIGHT});
        } else
        {
            return Direction::TOP;
        }
    }

    return utils::random_of( {Direction::LEFT,
                              Direction::BOTTOM,
                              Direction::RIGHT,
                              Direction::TOP});
}

} // ! anonymous namespace

void
TickDumbBot( Model&       model,
             const Snake& snake)
{
    if ( model.GetRabbits().empty() )
    {
        return ;
    }
    const Point& head = snake.points.back();

    Point target = get_closest_target( model, snake);

    model.SetSnakeDirection( snake.id, get_direction( head, target, snake.direction));
}

void
TickSmartBot( Model&       model,
              const Snake& snake)
{
    if ( model.GetRabbits().empty() )
    {
        return ;
    }

    std::queue<Point> workqueue{};
    std::unordered_set<Point, PointHash> visited{};
    std::unordered_map<Point, Point, PointHash> parents{};

    const Point& head = snake.points.back();
    workqueue.push( head);
    visited.insert( head);

    while ( !workqueue.empty() )
    {
        const Point& point = workqueue.front();
        workqueue.pop();

        if ( model.GetCellType( point) == CellType::RABBIT )
        {
            Point step = point;
            while ( parents[step] != head )
            {
                step = parents[step];
            }

            model.SetSnakeDirection( snake.id,
                                     get_direction( head, step, snake.direction));
            break;
        }

        std::array<Point, 4> neighbors{{
            { point.x + 1, point.y + 0},
            { point.x + 0, point.y + 1},
            { point.x - 1, point.y + 0},
            { point.x + 0, point.y - 1},
        }};
        auto winsz = model.GetFieldSize();
        for ( const Point& next : neighbors )
        {
            if ( (next.x < 0) ||
                 (next.x >= winsz.first) ||
                 (next.y < 0) ||
                 (next.y >= winsz.second) )
            {
                continue;
            }
            if ( (visited.count( next) == 0) &&
                 ( (model.GetCellType( next) == CellType::EMPTY) ||
                   (model.GetCellType( next) == CellType::RABBIT) ) )
            {
                workqueue.push( next);
                parents[next] = point;
                visited.insert( next);
            }
        }
    }
}

} // ! namespace bots
} // ! namespace snake
