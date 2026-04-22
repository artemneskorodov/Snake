#ifndef SNAKE_GAME_SETTINGS_HH__
#define SNAKE_GAME_SETTINGS_HH__

#include <string>
#include <vector>
#include <array>
#include <variant>

namespace snake
{
namespace settings
{

struct Button
{
    bool is_pressed;

};

struct SnakeSetting
{
    SnakeSetting( std::string name,
                  std::string color)
     :  name{ std::move( name)},
        color{ std::move( color)}
    {
    }

    enum class Active
    {
        NAME,
        COLOR
    };

    std::string name;
    std::string color;
    Active active;
    bool is_active{ false};
};

struct SnakesList
{
    static constexpr std::size_t kNoActive = SIZE_T_MAX;

    std::vector<SnakeSetting> snakes;
    std::size_t active { kNoActive};

};

struct MenuElement
{
    std::string name;
    std::variant<Button, SnakesList> element;
    bool is_active{ false};

};

class Menu
{
public:
    static constexpr std::size_t kMenuElementsNumber = 5;
    static constexpr std::size_t kNoActive = kMenuElementsNumber + 1;

    using MenuT = std::array<MenuElement, kMenuElementsNumber>;

public:
    void
    ActiveNext()
    {
        if ( active_ == kNoActive )
        {
            active_global_next();
        } else
        {
            if ( std::holds_alternative<Button>( menu_[active_].element) )
            {
                active_global_next();
            } else if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
            {
                SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
                if ( snakes_list.active == SnakesList::kNoActive )
                {
                    if ( snakes_list.snakes.empty() )
                    {
                        active_global_next();
                    } else
                    {
                        snakes_list.active = 0;
                        snakes_list.snakes[snakes_list.active].is_active = true;
                    }
                } else if ( (snakes_list.active + 1 == snakes_list.snakes.size()) )
                {
                    snakes_list.snakes[snakes_list.active].is_active = false;
                    active_global_next();
                } else
                {
                    snakes_list.snakes[snakes_list.active].is_active = false;
                    ++snakes_list.active;
                    snakes_list.snakes[snakes_list.active].is_active = true;
                }
            }
        }
    }

    void
    ActivePrev()
    {
        if ( active_ == kNoActive )
        {
            active_ = 0;
        } else
        {
            if ( std::holds_alternative<Button>( menu_[active_].element) )
            {
                active_global_prev();
            } else if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
            {
                SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
                if ( snakes_list.active == SnakesList::kNoActive )
                {
                    active_global_prev();
                } else if ( snakes_list.active == 0 )
                {
                    snakes_list.snakes[snakes_list.active].is_active = false;
                    snakes_list.active = SnakesList::kNoActive;
                } else
                {
                    snakes_list.snakes[snakes_list.active].is_active = false;
                    --snakes_list.active;
                    snakes_list.snakes[snakes_list.active].is_active = true;
                }
            }
        }
    }

    void
    InteractLeft()
    {
        if ( active_ == kNoActive )
        {
            return ;
        }

        if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
        {
            SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
            if ( snakes_list.active == SnakesList::kNoActive )
            {
                if ( !snakes_list.snakes.empty() )
                {
                    snakes_list.snakes.pop_back();
                }
            } else
            {
                SnakeSetting& snake = snakes_list.snakes[snakes_list.active];
                if ( snake.active == SnakeSetting::Active::COLOR )
                {
                    snake.active = SnakeSetting::Active::NAME;
                }
            }
        }
    }

    void
    InteractRight()
    {
        if ( active_ == kNoActive )
        {
            return ;
        }

        if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
        {
            SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
            if ( snakes_list.active == SnakesList::kNoActive )
            {
                snakes_list.snakes.emplace_back( "Snake" + std::to_string( current_snake_id_++),
                                                 "#3cef00");
            } else
            {
                SnakeSetting& snake = snakes_list.snakes[snakes_list.active];
                if ( snake.active == SnakeSetting::Active::NAME )
                {
                    snake.active = SnakeSetting::Active::COLOR;
                }
            }
        }
    }

    void
    InteractSelect()
    {
        if ( active_ == kNoActive )
        {
            return ;
        }

        if ( std::holds_alternative<Button>( menu_[active_].element) )
        {
            Button& button = std::get<Button>( menu_[active_].element);
            button.is_pressed = true;
        }
    }

    void
    InteractBackspace()
    {
        if ( active_ == kNoActive )
        {
            return ;
        }
        if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
        {
            SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
            if ( snakes_list.active != SnakesList::kNoActive )
            {
                SnakeSetting& snake = snakes_list.snakes[snakes_list.active];
                if ( snake.active == SnakeSetting::Active::NAME )
                {
                    if ( !snake.name.empty() )
                    {
                        snake.name.pop_back();
                    }
                } else if ( snake.active == SnakeSetting::Active::COLOR )
                {
                    if ( !snake.color.empty() )
                    {
                        snake.color.pop_back();
                    }
                }
            }
        }
    }

    void
    InteractSymbol( char c)
    {
        if ( active_ == kNoActive )
        {
            return ;
        }
        if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
        {
            SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
            if ( snakes_list.active != SnakesList::kNoActive )
            {
                SnakeSetting& snake = snakes_list.snakes[snakes_list.active];
                if ( snake.active == SnakeSetting::Active::NAME )
                {
                    snake.name += c;
                } else if ( snake.active == SnakeSetting::Active::COLOR )
                {
                    snake.color += c;
                }
            }
        }
    }

    const Button&
    GetExitBtn() const &
    {
        return std::get<Button>( menu_[0].element);
    }

    const SnakesList&
    GetHumanSnakes() const &
    {
        return std::get<SnakesList>( menu_[1].element);
    }

    const SnakesList&
    GetDumbBotSnakes() const &
    {
        return std::get<SnakesList>( menu_[2].element);
    }

    const SnakesList&
    GetSmartBotSnakes() const &
    {
        return std::get<SnakesList>( menu_[3].element);
    }

    const Button&
    GetStartGameBtn() const &
    {
        return std::get<Button>( menu_[4].element);
    }

    const MenuT&
    GetMenu() const &
    {
        return menu_;
    }

    std::size_t
    GetNumberBeforeActive() const
    {
        std::size_t result = 0;
        auto it = menu_.begin();

        while ( !it->is_active )
        {
            if ( std::holds_alternative<Button>( it->element) )
            {
                ++result;
            } else if ( std::holds_alternative<SnakesList>( it->element) )
            {
                const SnakesList& snakes_list = std::get<SnakesList>( it->element);
                result += 1 + snakes_list.snakes.size();
            }
            ++it;
        }
        if ( std::holds_alternative<SnakesList>( it->element) )
        {
            const SnakesList& snakes_list = std::get<SnakesList>( it->element);
            if ( snakes_list.active != snakes_list.kNoActive )
            {
                result += 1 + snakes_list.active;
            }
        }
        return result;
    }

private:
    void
    active_global_next()
    {
        if ( active_ == kNoActive )
        {
            active_ = 0;
            menu_[active_].is_active = true;
        } else if ( active_ + 1 != kMenuElementsNumber )
        {
            menu_[active_].is_active = false;
            ++active_;
            menu_[active_].is_active = true;
        }
    }

    void
    active_global_prev()
    {
        if ( active_ == kNoActive )
        {
            active_ = 0;
            menu_[active_].is_active = true;
            if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
            {
                SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
                if ( !snakes_list.snakes.empty() )
                {
                    snakes_list.snakes.back().is_active = true;
                }
            }
        } else if ( active_ != 0 )
        {
            menu_[active_].is_active = false;
            --active_;
            menu_[active_].is_active = true;
            if ( std::holds_alternative<SnakesList>( menu_[active_].element) )
            {
                SnakesList& snakes_list = std::get<SnakesList>( menu_[active_].element);
                if ( !snakes_list.snakes.empty() )
                {
                    snakes_list.snakes.back().is_active = true;
                }
            }
        }
    }

private:
    MenuT menu_{{
        {             "Exit",     Button{}},
        {     "Human Snakes", SnakesList{}},
        {  "Dumb Bot Snakes", SnakesList{}},
        { "Smart Bot Snakes", SnakesList{}},
        {       "Start Game",     Button{}}
    }};

    std::size_t active_           { kNoActive};
    std::size_t current_snake_id_ { 0};

};

} // ! namespace settings
} // ! namespace snake

#endif // ! SNAKE_GAME_SETTINGS_HH__
