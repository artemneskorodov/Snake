#include <iostream>
#include <string>

#include "simulation.hh"
#include "arguments.hh"
#include "model.hh"
#include "controller.hh"
#include "bots.hh"

namespace snake
{
namespace simulation
{

namespace
{

void
write_simulation_results( std::string         simulation_name,
                          const snake::Model& model)
{
    std::cout << simulation_name << ":" << std::endl;

    for ( const snake::Snake& snake : model.GetSnakes() )
    {
        std::cout << "Snake[" << snake.id << "] (";
        switch ( model.GetSnakeGroup( snake.id) )
        {
            case snake::SnakeGroup::DUMB:
            {
                std::cout << "dumb";
                break;
            }
            case snake::SnakeGroup::SMART:
            {
                std::cout << "smart";
                break;
            }
            default:
            {
                throw std::runtime_error{ "Unexpected snake group"};
            }
        }
        std::cout << "): scores=" << snake.GetScores() << std::endl;
    }
}

} // ! anonymous namespace

void
RunSimulation( const ProgramArguments& arguments)
{
    std::size_t dumb  = arguments.simulation_dumb_bots;
    std::size_t smart = arguments.simulation_smart_bots;
    std::size_t runs  = arguments.simulation_runs;

    for ( std::size_t i = 0; i != runs; ++i )
    {
        snake::Model model{};
        model.SetFieldSize( 15, 15);

        for ( std::size_t d = 0; d != dumb; ++d )
        {
            model.AddSnake( "",
                            snake::colors::Color{ "#ffffff"},
                            snake::SnakeGroup::SMART,
                            snake::bots::TickDumbBot);
        }

        for ( std::size_t s = 0; s != smart; ++s )
        {
            model.AddSnake( "",
                            snake::colors::Color{ "#ffffff"},
                            snake::SnakeGroup::SMART,
                            snake::bots::TickSmartBot);
        }

        for ( ; ; )
        {
            model.Tick();
            if ( model.GameFinished() )
            {
                break;
            }
        }

        write_simulation_results( "Simulation [" + std::to_string( i) + "]", model);
    }
}

} // ! namespace simulation
} // ! namespace snake
