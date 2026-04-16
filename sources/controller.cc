#include <optional>
#include <thread>
#include <chrono>

#include "model.hh"
#include "controller.hh"
#include "bots.hh"

namespace snake
{

void
Controller::Run( const ProgramArguments& arguments)
{
    for ( std::size_t player = 0; player != arguments.humans; ++player )
    {
        SnakeID snake_id = model_.AddSnake();
        players_snakes_.emplace_back( snake_id);
    }
    for ( std::size_t dumb_bot = 0; dumb_bot != arguments.dumb_bots; ++dumb_bot )
    {
        model_.AddSnake( bots::TickDumbBot);
    }
    for ( std::size_t smart_bot = 0; smart_bot != arguments.smart_bots; ++smart_bot )
    {
        model_.AddSnake( bots::TickSmartBot);
    }

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
            auto winsize = view_.GetGameFieldSize();
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
