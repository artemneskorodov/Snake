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

        bool need_pop_front = true;
        snake.points.emplace_back( next_head);

        for ( Rabbit& rabbit : rabbits_ )
        {
            if ( next_head == rabbit.point )
            {
                rabbit.is_alive = false;
                need_pop_front = false;
                break;
            }
        }
        if ( need_pop_front )
        {
            snake.points.pop_front();
        }
    }
}

} // ! namespace snake
