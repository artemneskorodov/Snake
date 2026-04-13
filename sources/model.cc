#include "model.hh"

namespace snake
{

void
Model::Tick()
{
    for ( Snake& snake : snakes_ )
    {
        Point next_head = snake.points.back();
        switch ( snake.direction )
        {
            case Direction::TOP:
            {
                --next_head.y;
                break;
            }
            case Direction::LEFT:
            {
                --next_head.x;
                break;
            }
            case Direction::BOTTOM:
            {
                ++next_head.y;
                break;
            }
            case Direction::RIGHT:
            {
                ++next_head.x;
                break;
            }
            default:
            {
                throw std::runtime_error{ "Unexpected snake direction"};
            }
        }
        snake.points.pop_front();
        snake.points.emplace_back( next_head);
    }
}

} // ! namespace snake
