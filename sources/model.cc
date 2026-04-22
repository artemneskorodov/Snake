#include "model.hh"

namespace snake
{

void
Model::Tick()
{
    for ( Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
        snake.ticker( *this, snake);
    }

    tick_snake_positions_update();
    tick_snake_rabbit_collisions_check();
    tick_snake_snake_collisions_check();
    tick_check_bones_lifetime();
    tick_snake_bone_collisions_check();

    if ( snakes_number_ == 0 )
    {
        game_finished_ = true;
    }

    if ( rabbits_counter_ == next_rabbit_counter_ )
    {
        rabbits_counter_ = 0;
        next_rabbit_counter_ = utils::random_normal( kRabbitsSpawnRateAvg,
                                                     kRabbitsSpawnRateSigma);
        add_rabbit();
    } else
    {
        ++rabbits_counter_;
    }

    ++tick_;
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

        if ( (next_head.x < 0) ||
             (next_head.x >= width_) ||
             (next_head.y < 0) ||
             (next_head.y >= height_) )
        {
            remove_snake( snake);
        }

        if ( snake.is_alive )
        {
            snake.points.emplace_back( next_head);
            cells_[next_head] = CellType::SNAKE;
            if ( view_update_callbacks_.snake_head_push_cb )
            {
                view_update_callbacks_.snake_head_push_cb( snake);
            }
        }
    }
}

void
Model::tick_snake_rabbit_collisions_check()
{
    for ( Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
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
            const Point& tail = snake.points.front();
            cells_[tail] = CellType::EMPTY;
            if ( view_update_callbacks_.removed_point_cb )
            {
                view_update_callbacks_.removed_point_cb( tail);
            }
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
            for ( ; it != std::prev( concurent.points.cend()); ++it )
            {
                if ( *it == head )
                {
                    remove_snake( snake);
                    break;
                }
            }

            if ( !snake.is_alive )
            {
                break;
            }

            if ( (&snake != &concurent) &&
                 (*it == head) )
            {
                remove_snake( snake);
                remove_snake( concurent);
                break;
            }
        }
    }
}

void
Model::tick_check_rabbits()
{
    for ( Rabbit& rabbit : rabbits_ )
    {
        if ( (rabbit.point.x < 0) ||
             (rabbit.point.x >= width_) ||
             (rabbit.point.y < 0) ||
             (rabbit.point.y >= height_) )
        {
            rabbit.is_alive = false;
            if ( view_update_callbacks_.removed_point_cb )
            {
                view_update_callbacks_.removed_point_cb( rabbit.point);
            }
        }
    }
}

void
Model::set_cells_after_resize()
{
    for ( Coordinate x = 0; x != width_; ++x )
    {
        for ( Coordinate y = 0; y != height_; ++y )
        {
            cells_[{ x, y}] = CellType::EMPTY;
        }
    }

    for ( const Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
        for ( const Point& point : snake.points )
        {
            cells_[point] = CellType::SNAKE;
        }
    }

    for ( const Rabbit& rabbit : rabbits_ )
    {
        if ( !rabbit.is_alive )
        {
            continue;
        }
        cells_[rabbit.point] = CellType::RABBIT;
    }
}

void
Model::remove_snake( Snake& snake)
{
    snake.is_alive = false;
    for ( const Point& point : snake.points )
    {
        bool spawn_bone = utils::random_true_false( kBoneSpawnProbability);
        if ( spawn_bone )
        {
            TickType lifetime = utils::random_normal( kBonesLifetimeAvg, kBonesLifetimeSigma);
            add_bone( point, lifetime);
        } else
        {
            cells_[point] = CellType::EMPTY;
        }
    }
    if ( view_update_callbacks_.removed_point_cb )
    {
        for ( const Point& point : snake.points )
        {
            view_update_callbacks_.removed_point_cb( point);
        }
    }
    --snakes_number_;
}

void
Model::add_rabbit()
{
    Coordinate x;
    Coordinate y;
    int counter = 0;
    for ( ; counter != 10; ++counter )
    {
        x = utils::random_min_max( 0, width_ - 1);
        y = utils::random_min_max( 0, height_ - 1);
        if ( cells_[{ x, y}] == CellType::EMPTY )
        {
            break;
        }
    }
    if ( counter == 10 )
    {
        return ;
    }
    rabbits_.emplace_back( x, y);
    cells_[{ x, y}] = CellType::RABBIT;

    if ( view_update_callbacks_.rabbit_add_cb )
    {
        view_update_callbacks_.rabbit_add_cb( { x, y});
    }
}

SnakeID
Model::AddSnake( std::string   name,
                 colors::Color color,
                 SnakeGroup    group,
                 SnakeTicker   ticker)
{
    SnakeID id = static_cast<SnakeID>( snakes_.size());
    Direction dir = utils::random_of( { Direction::LEFT,
                                       Direction::BOTTOM,
                                       Direction::RIGHT,
                                       Direction::TOP});

    Coordinate x;
    Coordinate y;

    for ( ; ; )
    {
        x = utils::random_min_max( 0, width_ - 1);
        y = utils::random_min_max( 0, height_ - 1);
        if ( cells_[{ x, y}] == CellType::EMPTY )
        {
            break;
        }
    }
    snakes_.emplace_back( x, y, dir, id, ticker, std::move( name), color);

    // Adding one more snake cell
    Point head = snakes_.back().points.back();
    switch ( dir )
    {
        case Direction::TOP:
        {
            head.y -= 1;
            break;
        }
        case Direction::LEFT:
        {
            head.x -= 1;
            break;
        }
        case Direction::BOTTOM:
        {
            head.y += 1;
            break;
        }
        case Direction::RIGHT:
        {
            head.x += 1;
            break;
        }
        default:
        {
            throw std::runtime_error{ "Unexpected direction"};
        }
    }
    snakes_.back().points.emplace_back( head);

    cells_[{ x, y}] = CellType::SNAKE;
    cells_[head]    = CellType::SNAKE;

    ++snakes_number_;

    switch ( group )
    {
        case SnakeGroup::HUMAN:
        {
            humans_snakes_group_.emplace_back( id);
            break;
        }
        case SnakeGroup::DUMB:
        {
            dumb_snakes_group_.emplace_back( id);
            break;
        }
        case SnakeGroup::SMART:
        {
            smart_snakes_group_.emplace_back( id);
            break;
        }
        default:
        {
            throw std::runtime_error{ "Unexpected snakes group"};
        }
    }

    return id;
}

void
Model::add_bone( const Point& point,
                 TickType     lifetime)
{
    cells_[point] = CellType::BONE;
    bones_.emplace_back( point.x, point.y, lifetime + tick_);
    if ( view_update_callbacks_.bone_add_cb )
    {
        view_update_callbacks_.bone_add_cb( point);
    }
}

void
Model::tick_check_bones_lifetime()
{
    for ( Bone& bone : bones_ )
    {
        if ( tick_ == bone.death_tick )
        {
            bone.is_alive = false;
            cells_[bone.point] = CellType::EMPTY;
            if ( view_update_callbacks_.bone_add_cb )
            {
                view_update_callbacks_.removed_point_cb( bone.point);
            }
        }
    }
}

void
Model::tick_snake_bone_collisions_check()
{
    for ( Snake& snake : snakes_ )
    {
        if ( !snake.is_alive )
        {
            continue;
        }
        const Point& head = snake.points.back();
        for ( const Bone& bone : bones_ )
        {
            if ( !bone.is_alive )
            {
                continue;
            }
            if ( head == bone.point )
            {
                remove_snake( snake);
                break;
            }
        }
    }
}

} // ! namespace snake
