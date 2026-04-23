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

struct SnakeBotInfo
{
    std::string_view name;
    SnakeTicker ticker;
};

const std::array<SnakeBotInfo, 2> kSnakeBots{{
    { "Dumb", bots::TickDumbBot},
    { "Smart", bots::TickSmartBot}
}};

} // ! anonymous namespace

void
RunSimulation( const ProgramArguments& arguments)
{
    std::size_t runs_number = arguments.simulation_runs;
    // PvE: bots alone on field eating rabbits

    for ( std::size_t run = 0; run != runs_number; ++run )
    {
        // Setting simulation seed
        uint32_t seed = run;
        utils::random::SetSeed( seed);

        std::cout << "Simulation [" << run << "] (seed = " << seed << "):" << std::endl;

        for ( const SnakeBotInfo& info : kSnakeBots )
        {
            Model model = run_pve_simulation( info.ticker);
            std::cout << "    "
                      << std::setw( 10) << std::left << info.name
                      << std::setw( 10) << std::right << model.GetSnakes().back().GetScores()
                      << std::endl;
        }
    }
}

} // ! namespace simulation
} // ! namespace snake
