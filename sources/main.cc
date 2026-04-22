#include "ascii_view.hh"
#include "graphics_view.hh"
#include "controller.hh"
#include "model.hh"
#include "arguments.hh"
#include "simulation.hh"

int
main( int         argc,
      const char *argv[])
{
    snake::ProgramArguments arguments = snake::GetProgramArguments( argc, argv);

    if ( arguments.simulate )
    {
        snake::simulation::RunSimulation( arguments);
        return EXIT_SUCCESS;
    }

    std::unique_ptr<snake::View> view = nullptr;
    if ( arguments.graphics_type == snake::GraphicsType::ASCII )
    {
        view = std::make_unique<snake::AsciiView>();
    } else if ( arguments.graphics_type == snake::GraphicsType::SFML )
    {
        view = std::make_unique<snake::GraphicsView>( arguments.window_size.first,
                                                      arguments.window_size.second);
    } else
    {
        throw std::runtime_error{ "Unexpected view type"};
    }

    snake::Model model{};

    snake::Controller controller{ model, *view};
    controller.Run();
}
