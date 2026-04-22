#include <string>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include "arguments.hh"

namespace snake
{

namespace
{

struct ArgInfo
{
    std::string_view                                            long_name;
    std::string_view                                            short_name;
    std::string_view                                            description;
    std::function<void( ProgramArguments&, const std::string&)> reader;
    std::function<void( ProgramArguments&)>                     default_setter;
};

const std::array<ArgInfo, 8> kArgsInfo
{{
    { "--graphics", "-g",
      "Type of graphics: sfml or ascii. Usage: --graphics=<sfml/ascii>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            if ( string == "=sfml")
            {
                arguments.graphics_type = GraphicsType::SFML;
            } else if ( string == "=ascii" )
            {
                arguments.graphics_type = GraphicsType::ASCII;
            } else
            {
                throw std::runtime_error{ "Unexpected string representing graphics type: " + string};
            }
        },
      []( ProgramArguments& arguments)
        {
            arguments.graphics_type = GraphicsType::ASCII;
        }
    },

    { "--screen-width", "-sw",
      "Width of window, used if only graphics type is sfml. Usage: --width=<number>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            arguments.window_size.first = std::stoul( string.c_str() + 1, nullptr, 10);
        },
      []( ProgramArguments& arguments)
        {
            arguments.window_size.first = 1500;
        }
    },

    { "--screen-height", "-sh",
      "Height of window, used if only graphics type is sfml. Usage: --height=<number>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            arguments.window_size.second = std::stoul( string.c_str() + 1, nullptr, 10);
        },
      []( ProgramArguments& arguments)
        {
            arguments.window_size.second = 1000;
        }
    },

    { "--help", "-h",
      "Show this information. Usage: --help",
      []( ProgramArguments& , const std::string& )
        {
            for ( const ArgInfo& info : kArgsInfo )
            {
                std::cout << std::left << std::setw( 15) << info.long_name << " ( "
                          << std::left << std::setw( 5)  << info.short_name << " )    "
                          << std::left << info.description << std::endl;
            }
            std::cout << "Press any key to continue...";
            std::getchar();
        },
      []( ProgramArguments& )
        {
        }
    },

    { "--simulate", "-sim",
      "Run simulation without graphics to collect statistics. Usage: --help",
      []( ProgramArguments& , const std::string& )
        {
        },
      []( ProgramArguments& arguments)
        {
            arguments.simulate = false;
        }
    },

    { "--simulate-dumb", "-sim-d",
      "Number of dumb bots in simulation. Usage: --simulate-dumb=<number>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            arguments.simulation_dumb_bots = std::stoul( string.c_str() + 1, nullptr, 10);
        },
      []( ProgramArguments& arguments)
        {
            arguments.simulation_dumb_bots = 3;
        }
    },

    { "--simulate-smart", "-sim-s",
      "Number of smart bots in simulation. Usage: --simulate-smart=<number>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            arguments.simulation_smart_bots = std::stoul( string.c_str() + 1, nullptr, 10);
        },
      []( ProgramArguments& arguments)
        {
            arguments.simulation_smart_bots = 3;
        }
    },

    { "--simulate-runs", "-sim-r",
      "Number of runs to simulate. Usage: --simulate-runs=<number>",
      []( ProgramArguments& arguments, const std::string& string)
        {
            arguments.simulation_runs = std::stoul( string.c_str() + 1, nullptr, 10);
        },
      []( ProgramArguments& arguments)
        {
            arguments.simulation_runs = 3;
        }
    }
}};

} // ! anonymous namespace

ProgramArguments
GetProgramArguments( int argc, const char *argv[])
{
    std::array<bool, kArgsInfo.size()> used{};

    ProgramArguments arguments;
    for ( int i = 1; i != argc; ++i )
    {
        std::string argument = std::string( argv[i]);
        bool match = false;
        for ( std::size_t arg_id = 0; arg_id != kArgsInfo.size(); ++arg_id)
        {
            const ArgInfo& info = kArgsInfo[arg_id];
            std::size_t short_length = info.short_name.length();
            std::size_t long_length  = info.long_name.length();

            if ( (argument.length() >= short_length) &&
                 (argument.substr( 0, short_length) == info.short_name) )
            {
                info.reader( arguments, argument.substr( short_length));
                match = true;
                used[arg_id] = true;
                break;
            } else if ( (argument.length() >= long_length) &&
                        (argument.substr(0, long_length) == info.long_name) )
            {
                info.reader( arguments, argument.substr( long_length));
                match = true;
                used[arg_id] = true;
                break;
            }
        }
        if ( !match )
        {
            throw std::runtime_error{ "Unknown argument: " + argument};
        }
    }

    for ( std::size_t i = 0; i != kArgsInfo.size(); ++i )
    {
        if ( !used[i] )
        {
            kArgsInfo[i].default_setter( arguments);
        }
    }
    return arguments;
}

} // ! namespace snake
