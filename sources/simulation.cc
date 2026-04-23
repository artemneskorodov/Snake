#include <iostream>
#include <string>
#include <sstream>

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
run_single_pve_simulation( SnakeTicker ticker)
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
run_single_pvp_simulation( SnakeTicker first,
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
    SnakeTicker      ticker;
};

const std::array<SnakeBotInfo, 2> kSnakeBots{{
    { "Dumb",  bots::TickDumbBot  },
    { "Smart", bots::TickSmartBot }
}};

struct PvEResult
{
    int         scores;
    std::size_t bot_info_id;
};

struct PvPResult
{
    int         scores_first;
    int         scores_second;
    std::size_t bot_info_id_first;
    std::size_t bot_info_id_second;
};

template<typename ResultT>
struct Simulation
{
    std::vector<ResultT> results;
    std::size_t          run;
    std::uint32_t        seed;
};

std::vector<Simulation<PvEResult>>
run_pve_simulations( std::size_t runs_number)
{
    std::vector<Simulation<PvEResult>> results{};

    for ( std::size_t run = 0; run != runs_number; ++run )
    {
        results.emplace_back();
        Simulation<PvEResult>& result = results.back();

        result.run = run;

        uint32_t seed = run;
        utils::random::SetSeed( seed);
        result.seed = seed;

        for ( std::size_t bot = 0; bot != kSnakeBots.size(); ++bot )
        {
            const SnakeBotInfo& info = kSnakeBots[bot];

            Model model = run_single_pve_simulation( info.ticker);

            int scores = model.GetSnake( 0).GetScores();

            result.results.emplace_back( PvEResult{ scores, bot});
        }
    }
    return results;
}

std::vector<Simulation<PvPResult>>
run_pvp_simulations( std::size_t runs_number)
{
    std::vector<Simulation<PvPResult>> results{};
    for ( std::size_t run = 0; run != runs_number; ++run )
    {
        Simulation<PvPResult>& result = results.emplace_back();
        result.run = run;

        // Setting simulation seed
        uint32_t seed = run;
        utils::random::SetSeed( seed);
        result.seed = seed;

        for ( std::size_t bot_first = 0; bot_first + 1 != kSnakeBots.size(); ++bot_first )
        {
            for ( std::size_t bot_second = bot_first + 1; bot_second != kSnakeBots.size(); ++bot_second )
            {
                const SnakeBotInfo& info_first  = kSnakeBots[bot_first];
                const SnakeBotInfo& info_second = kSnakeBots[bot_second];

                Model model = run_single_pvp_simulation( info_first.ticker, info_second.ticker);
                int scores_first  = model.GetSnake( 0).GetScores();
                int scores_second = model.GetSnake( 1).GetScores();

                result.results.emplace_back( PvPResult{ scores_first,
                                                              scores_second,
                                                              bot_first,
                                                              bot_second});
            }
        }
    }
    return results;
}

} // ! anonymous namespace

void
RunSimulation( const ProgramArguments& arguments)
{
    std::size_t runs_number = arguments.simulate;

    std::stringstream json_results{};
    json_results << "{\n";

    json_results << "\t" "\"pve\":[\n";
    if ( arguments.simulate_pve )
    {
        std::vector<Simulation<PvEResult>> simulations = run_pve_simulations( runs_number);

        for ( const Simulation<PvEResult>& sim : simulations )
        {
            json_results << "\t\t" "{\n"
                            "\t\t" "\"run\": " << sim.run << ",\n"
                            "\t\t" "\"seed\": " << sim.seed << ",\n"
                            "\t\t" "\"results\": [\n";

            for ( const PvEResult& res : sim.results )
            {
                const SnakeBotInfo& info = kSnakeBots[res.bot_info_id];
                json_results << "\t\t\t" "{\"name\": \"" << info.name << "\", "
                                "\"score\": " << res.scores << "}"
                                << ((&res != &sim.results.back()) ? "," : "") << "\n";
            }

            json_results << "\t\t\t" "]\n"
                         << "\t\t" "}"
                         << ((&sim != &simulations.back()) ? "," : "") << "\n";
        }
    }
    json_results << "\t" "],\n"
                    "\t" "\"pvp\":[\n";
    if ( arguments.simulate_pvp )
    {
        std::vector<Simulation<PvPResult>> simulations = run_pvp_simulations( runs_number);

        for ( const Simulation<PvPResult>& sim : simulations )
        {
            json_results << "\t\t" "{\n"
                            "\t\t" "\"run\": " << sim.run << ",\n"
                            "\t\t" "\"seed\": " << sim.seed << ",\n"
                            "\t\t" "\"results\": [\n";

            for ( const PvPResult& res : sim.results )
            {
                const SnakeBotInfo& info_first  = kSnakeBots[res.bot_info_id_first];
                const SnakeBotInfo& info_second = kSnakeBots[res.bot_info_id_second];

                json_results << "\t\t\t" "{\"first-name\": \"" << info_first.name << "\", "
                                "\"second-name\": \"" << info_second.name << "\", "
                                "\"first-score\": " << res.scores_first << ","
                                "\"second-score\": " << res.scores_second << "}"
                                << ((&res != &sim.results.back()) ? "," : "") << "\n";
            }

            json_results << "\t\t\t" "]\n"
                         << "\t\t" "}"
                         << ((&sim != &simulations.back()) ? "," : "") << "\n";
        }
    }
    json_results << "]\n"
                    "}";

    // std::cout << ss.str();
    std::cout << json_results.str();
}

} // ! namespace simulation
} // ! namespace snake
