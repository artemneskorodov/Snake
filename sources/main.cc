#include "ascii_view.hh"
#include "controller.hh"
#include "model.hh"

int
main( void)
{
    std::srand( std::time( nullptr));
    snake::AsciiView view{};

    view.RequestWindowSize();
    auto winsz = view.GetWindowSize();
    snake::Model model{ winsz.first, winsz.second};

    snake::Controller controller{ model, view};
    controller.Run();
}
