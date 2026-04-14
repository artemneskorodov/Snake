#include <optional>
#include <thread>
#include <chrono>

#include "controller.hh"

namespace snake
{

namespace
{

template<typename T>
T
rand_min_max( T min,
              T max)
{
    return (std::rand() % (max - min)) + min;
}

constexpr int kMinRabbitsSpawnRate = 10;
constexpr int kMaxRabbitsSpawnRate = 20;

} // ! anonymous namespace

void
Controller::Run()
{
    SnakeID first_snake_id  = model_.AddSnake( 10, 10, Direction::TOP);
    SnakeID second_snake_id = model_.AddSnake( 20, 20, Direction::TOP);
    players_snakes_.emplace_back( first_snake_id);
    players_snakes_.emplace_back( second_snake_id);

    int rabbits_counter     = 0;
    int next_rabbit_counter = rand_min_max( kMinRabbitsSpawnRate, kMaxRabbitsSpawnRate);

    for ( ; ; )
    {
        view_.UpdateEvents();
        for ( ; ; )
        {
            std::optional<Event> event = view_.PopEvent();
            if ( !event.has_value() )
            {
                break;
            }

            handle_event( event.value());
            if ( need_exit_ )
            {
                break;
            }
        }
        if ( need_exit_ )
        {
            break;
        }

        if ( rabbits_counter == next_rabbit_counter )
        {
            auto winsz = model_.GetFieldSize();
            Coordinate x = rand_min_max( 0, winsz.first);
            Coordinate y = rand_min_max( 0, winsz.second);
            model_.AddRabbit( x, y);

            rabbits_counter = 0;
            next_rabbit_counter = rand_min_max( kMinRabbitsSpawnRate, kMaxRabbitsSpawnRate);
        } else
        {
            ++rabbits_counter;
        }

        model_.Tick();

        view_.Render( model_);
        if ( model_.GameFinished() )
        {
            break;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 150));
    }

    players_snakes_.clear();
}

void
Controller::handle_event( Event event)
{
    switch ( event.event )
    {
        case Event::KEY_PRESSED_PLAYER_TOP:
        {
            model_.SetSnakeDirection( players_snakes_[event.player], Direction::TOP);
            break;
        }
        case Event::KEY_PRESSED_PLAYER_LEFT:
        {
            model_.SetSnakeDirection( players_snakes_[event.player], Direction::LEFT);
            break;
        }
        case Event::KEY_PRESSED_PLAYER_BOTTOM:
        {
            model_.SetSnakeDirection( players_snakes_[event.player], Direction::BOTTOM);
            break;
        }
        case Event::KEY_PRESSED_PLAYER_RIGHT:
        {
            model_.SetSnakeDirection( players_snakes_[event.player], Direction::RIGHT);
            break;
        }
        case Event::KEY_PRESSED_EXIT:
        {
            need_exit_ = true;
            break;
        }
        case Event::WINDOW_SIZE_CHANGED:
        {
            auto winsize = view_.GetWindowSize();
            model_.SetFieldSize( winsize.first, winsize.second);
            break;
        }
        default:
        {
            throw std::runtime_error{ "Unexpected event"};
        }
    }
}

} // ! namespace snake
