#ifndef CONTROLLER_HH__
#define CONTROLLER_HH__

#include "model.hh"
#include "view.hh"
#include "arguments.hh"
#include "game_settings.hh"

namespace snake
{

class Controller final
{
public:
    Controller( Model& model, View& view)
     :  model_{ model},
        view_{ view}
    {
    }

    void Run();

private:
    settings::Menu run_menu();
    void run_game( const settings::Menu& settings);
    void handle_game_event( Event event);
    void handle_menu_event( MenuEvent event, settings::Menu& menu);

private:
    static bool validate_snakes_colors( const settings::Menu& settings);

private:
    Model& model_;
    View& view_;
    bool need_exit_                      { false};
    bool need_go_to_menu_                { false};
    std::vector<SnakeID> players_snakes_ {};

};

} // ! namespace snake

#endif // ! CONTROLLER_HH__
