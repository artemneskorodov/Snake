#ifndef CONTROLLER_HH__
#define CONTROLLER_HH__

#include "model.hh"
#include "view.hh"

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
    void handle_event( Event event);

private:
    Model& model_;
    View& view_;
    bool need_exit_{ false};
    std::vector<SnakeID> players_snakes_;

};

} // ! namespace snake

#endif // ! CONTROLLER_HH__
