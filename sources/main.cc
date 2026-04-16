#include "ascii_view.hh"
#include "graphics_view.hh"
#include "controller.hh"
#include "model.hh"

int
main( void)
{
    snake::GraphicsView view{800, 600};

    view.RequestWindowSize();
    auto winsz = view.GetGameFieldSize();
    snake::Model model{ winsz.first, winsz.second};

    snake::Controller controller{ model, view};
    controller.Run();
}
