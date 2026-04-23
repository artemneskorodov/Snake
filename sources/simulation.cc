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

Model
run_pvp_simulation( SnakeTicker first,
                    SnakeTicker second)
{
    Model model{};
    model.SetFieldSize( kFieldWidth, kFieldHeight);

    model.AddSnake( "",
                    colors::Color{ "#000000"},
                    SnakeGroup::HUMAN,
                    first);

    model.AddSnake( "",
                    colors::Color{ "#000000"},
                    SnakeGroup::HUMAN,
                    second);

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

        std::cout << "PvE Simulation [" << run << "] (seed = " << seed << "):" << std::endl;

        for ( const SnakeBotInfo& info : kSnakeBots )
        {
            Model model = run_pve_simulation( info.ticker);
            int scores = model.GetSnake( 0).GetScores();

            std::cout << "\t"
                      << std::setw( 10) << std::left << info.name
                      << std::setw( 10) << std::right << scores
                      << std::endl;
        }
    }

    // PvP: all pairs of bots
    for ( std::size_t run = 0; run != runs_number; ++run )
    {
        // Setting simulation seed
        uint32_t seed = run;
        utils::random::SetSeed( seed);

        std::cout << "PvP Simulation [" << run << "] (seed = " << seed << "):" << std::endl;

        auto end = kSnakeBots.end();
        for ( auto it_first = kSnakeBots.begin(); it_first != std::prev( end); ++it_first )
        {
            for ( auto it_second = std::next( it_first); it_second != end; ++it_second )
            {
                Model model = run_pvp_simulation( it_first->ticker, it_second->ticker);
                int scores_first  = model.GetSnake( 0).GetScores();
                int scores_second = model.GetSnake( 1).GetScores();

                std::cout << "\t"
                          << it_first->name << " vs " << it_second->name << std::endl
                          << "\t\t"
                          << std::setw( 10) << std::right << it_first->name
                          << ".Scores = " << scores_first << std::endl
                          << "\t\t"
                          << std::setw( 10) << std::right << it_second->name
                          << ".Scores = " << scores_second << std::endl;
            }
        }
    }
}

} // ! namespace simulation
} // ! namespace snake
