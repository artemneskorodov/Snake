#ifndef SNAKE_VIEW_HH__
#define SNAKE_VIEW_HH__

#include <optional>
#include <queue>

#include "model.hh"
#include "game_settings.hh"

namespace snake
{

using PlayerID = uint32_t;

struct Event
{
    enum DetailEventEn
    {
        KEY_PRESSED_PLAYER_TOP    = 1,
        KEY_PRESSED_PLAYER_LEFT   = 2,
        KEY_PRESSED_PLAYER_BOTTOM = 3,
        KEY_PRESSED_PLAYER_RIGHT  = 4,
        KEY_PRESSED_EXIT          = 5,
        WINDOW_SIZE_CHANGED       = 6,
    };

    // Not explicit
    constexpr
    Event( DetailEventEn event)
     :  event{ event}
    {
    }

    constexpr
    Event( PlayerID      player,
           DetailEventEn event)
     :  player { player},
        event  { event}
    {
    }

    PlayerID      player{};
    DetailEventEn event;

};

enum MenuEvent
{
    KEY_PRESSED_ARROW_UP    = 1,
    KEY_PRESSED_ARROW_LEFT  = 2,
    KEY_PRESSED_ARROW_DOWN  = 3,
    KEY_PRESSED_ARROW_RIGHT = 4,
    KEY_PRESSED_ENTER       = 5,
    EXIT                    = 6,
    BACKSPACE               = 7,

    // TODO check that this enums are less than printed symbols
};

class View
{
public:
    virtual ~View() = default;

public:
    virtual void                              Render( const Model& model)                 = 0;
    virtual void                              RenderMenu( const settings::Menu& settings) = 0;
    virtual std::pair<Coordinate, Coordinate> GetGameFieldSize() const                    = 0;
    virtual void                              UpdateEvents()                              = 0;
    virtual void                              UpdateMenuEvents()                          = 0;

public:
    std::optional<Event>
    PopEvent()
    {
        if ( events_.empty() )
        {
            return std::nullopt;
        }

        Event event = events_.front();
        events_.pop();
        return event;
    }

    std::optional<MenuEvent>
    PopMenuEvent()
    {
        if ( menu_events_.empty() )
        {
            return std::nullopt;
        }

        MenuEvent event = menu_events_.front();
        menu_events_.pop();
        return event;
    }

protected:
    std::pair<Coordinate, Coordinate> current_window_size_{};
    std::queue<Event>                 events_{};
    std::queue<MenuEvent>             menu_events_{};

};

} // ! namespace snake

#endif // ! SNAKE_VIEW_HH__
