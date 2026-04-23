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

constexpr Coordinate kFieldWidth  = 25;
constexpr Coordinate kFieldHeight = 25;

Model
run_pve_simulation( SnakeTicker ticker)
{
    Model model{};
    model.SetFieldSize( kFieldWidth, kFieldHeight);

    model.AddSnake( "",
                    colors::Color{ "#000000"},
                    SnakeGroup::HUMAN,
                    ticker);

    for ( ; ; )
    {
        model.Tick();
        if ( model.GameFinished() )
        {
            break;
        }
    }

    return model;
}

} // ! anonymous namespace

void
RunSimulation( const ProgramArguments& arguments)
{
    std::size_t runs_number = arguments.simulation_runs;
    // PvE: bots alone on field eating rabbits
    for ( std::size_t run = 0; run != runs_number; ++run )
    {
        std::uint32_t seed = std::hash<std::uint32_t>{}( run);

        utils::random::SetSeed( seed);

        Model model_dumb = run_pve_simulation( snake::bots::TickDumbBot);
        Model model_smart = run_pve_simulation( snake::bots::TickSmartBot);

        std::cout << "Simulation [" << run << "] (width seed = " << seed << "):" << std::endl;
        std::cout << "Dumb:  " << model_dumb.GetSnake( 0).GetScores() << std::endl;
        std::cout << "Smart: " << model_smart.GetSnake( 0).GetScores() << std::endl;
    }
}

} // ! namespace simulation
} // ! namespace snake
