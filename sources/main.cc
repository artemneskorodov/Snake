#include "ascii_view.hh"
#include "graphics_view.hh"
#include "controller.hh"
#include "model.hh"
#include "arguments.hh"

int
main( int         argc,
      const char *argv[])
{
    snake::ProgramArguments arguments = snake::GetProgramArguments( argc, argv);

    std::unique_ptr<snake::View> view = nullptr;
    if ( arguments.graphics_type == snake::GraphicsType::ASCII )
    {
        view = std::make_unique<snake::AsciiView>();
    } else if ( arguments.graphics_type == snake::GraphicsType::SFML )
    {
        if ( (arguments.window_size.first == 0) ||
             (arguments.window_size.second == 0) )
        {
            throw std::runtime_error{ "It is expected to get width and height of window with "
                                      "--width=<number> and --height=<number> in graphics view"};
        }
        view = std::make_unique<snake::GraphicsView>( arguments.window_size.first,
                                                      arguments.window_size.second);
    } else
    {
        throw std::runtime_error{ "Unexpected view type"};
    }

    snake::Model model{};

    snake::Controller controller{ model, *view};
    controller.Run( arguments);
}
