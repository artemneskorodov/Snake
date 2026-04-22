#include <optional>
#include <thread>
#include <chrono>
#include <iostream>

#include "model.hh"
#include "controller.hh"
#include "bots.hh"
#include "game_settings.hh"

namespace snake
{

void
Controller::Run()
{
    for ( ; ; )
    {
        settings::Menu settings = run_menu();
        if ( need_exit_ )
        {
            break;
        }
        run_game( settings);
        need_go_to_menu_ = false;
    }
}

settings::Menu
Controller::run_menu()
{
    settings::Menu settings{};

    for ( ; ; )
    {
        view_.UpdateMenuEvents();
        for ( ; ; )
        {
            std::optional<MenuEvent> event = view_.PopMenuEvent();
            if ( !event.has_value() )
            {
                break;
            }

            handle_menu_event( event.value(), settings);
        }

        const settings::Button& exit_button = settings.GetExitBtn();
        if ( exit_button.is_pressed )
        {
            need_exit_ = true;
            break;
        }

        settings::Button& start_button = settings.GetStartGameBtn();
        if ( start_button.is_pressed )
        {
            if ( !validate_snakes_colors( settings) )
            {
                start_button.is_pressed = false;
            } else
            {
                break;
            }
        }

        view_.RenderMenu( settings);

        std::this_thread::sleep_for( std::chrono::milliseconds( 150));
    }
    return settings;
}

void
Controller::run_game( const settings::Menu& settings)
{
    // Resetting game
    auto winsz = view_.GetGameFieldSize();
    model_ = Model{};
    model_.SetFieldSize( winsz.first, winsz.second);
    players_snakes_.clear();

    const settings::SnakesList& human = settings.GetHumanSnakes();
    const settings::SnakesList& dumb  = settings.GetDumbBotSnakes();
    const settings::SnakesList& smart = settings.GetSmartBotSnakes();

    for ( const settings::SnakeSetting& snake : human.snakes )
    {
        colors::Color color{ snake.color};
        SnakeID snake_id = model_.AddSnake( snake.name, color, SnakeGroup::HUMAN);
        players_snakes_.emplace_back( snake_id);
    }
    for ( const settings::SnakeSetting& snake : dumb.snakes )
    {
        colors::Color color{ snake.color};
        model_.AddSnake( snake.name, color, SnakeGroup::DUMB, bots::TickDumbBot);
    }
    for ( const settings::SnakeSetting& snake : smart.snakes )
    {
        colors::Color color{ snake.color};
        model_.AddSnake( snake.name, color, SnakeGroup::SMART, bots::TickSmartBot);
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

            handle_game_event( event.value());
        }
        if ( need_go_to_menu_ )
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
}

void
Controller::handle_game_event( Event event)
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
            need_go_to_menu_ = true;
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

void
Controller::handle_menu_event( MenuEvent       event,
                               settings::Menu& menu)
{
    switch ( event )
    {
        case MenuEvent::KEY_PRESSED_ARROW_UP:
        {
            menu.ActivePrev();
            break;
        }
        case MenuEvent::KEY_PRESSED_ARROW_LEFT:
        {
            menu.InteractLeft();
            break;
        }
        case MenuEvent::KEY_PRESSED_ARROW_DOWN:
        {
            menu.ActiveNext();
            break;
        }
        case MenuEvent::KEY_PRESSED_ARROW_RIGHT:
        {
            menu.InteractRight();
            break;
        }
        case MenuEvent::KEY_PRESSED_ENTER:
        {
            menu.InteractSelect();
            break;
        }
        case MenuEvent::EXIT:
        {
            need_exit_ = true;
            break;
        }
        case MenuEvent::BACKSPACE:
        {
            menu.InteractBackspace();
            break;
        }
        default:
        {
            menu.InteractSymbol( static_cast<char>( event));
            break;
        }
    }
}

bool
Controller::validate_snakes_colors( const settings::Menu& settings)
{
    const settings::SnakesList& human = settings.GetHumanSnakes();
    const settings::SnakesList& dumb  = settings.GetDumbBotSnakes();
    const settings::SnakesList& smart = settings.GetSmartBotSnakes();

    for ( const settings::SnakeSetting& snake : human.snakes )
    {
        if ( !colors::IsValidColor( snake.color) )
        {
            return false;
        }
    }
    for ( const settings::SnakeSetting& snake : dumb.snakes )
    {
        if ( !colors::IsValidColor( snake.color) )
        {
            return false;
        }
    }
    for ( const settings::SnakeSetting& snake : smart.snakes )
    {
        if ( !colors::IsValidColor( snake.color) )
        {
            return false;
        }
    }
    return true;
}

} // ! namespace snake
