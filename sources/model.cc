#include "model.hh"

namespace snake
{

void
Model::Tick()
{
    tick_snake_positions_update();
    tick_snake_rabbit_collisions_check();
    tick_snake_snake_collisions_check();

    if ( snakes_number_ == 0 )
    {
        game_finished_ = true;
    }
}

void
Model::tick_snake_positions_update()
{
    for ( Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
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

        snake.points.emplace_back( next_head);
    }
}

void
Model::tick_snake_rabbit_collisions_check()
{
    for ( Snake& snake : snakes_ )
    {
        const Point& head = snake.points.back();
        bool need_pop_front = true;

        for ( Rabbit& rabbit : rabbits_ )
        {
            if ( !rabbit.is_alive )
            {
                continue;
            }
            if ( head == rabbit.point )
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

void
Model::tick_snake_snake_collisions_check()
{
    for ( Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
        const Point& head = snake.points.back();
        for ( Snake& concurent : snakes_ )
        {
            if ( !concurent.is_alive )
            {
                continue;
            }
            auto it = concurent.points.cbegin();
            for ( ; it != std::prev( concurent.points.cend()); it++ )
            {
                if ( *it == head )
                {
                    --snakes_number_;
                    snake.is_alive = false;
                    break;
                }
            }

            if ( &snake != &concurent )
            {
                if ( (&snake != &concurent) &&
                     (*it == head) )
                {
                    snakes_number_ -= 2;
                    snake.is_alive = false;
                    concurent.is_alive = false;
                    break;
                }
            }
        }
    }
}

} // ! namespace snake
