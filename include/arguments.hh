#ifndef SNAKE_ARGUMENTS_HH__
#define SNAKE_ARGUMENTS_HH__

#include <utility>

namespace snake
{

enum class GraphicsType
{
    ASCII,
    SFML,
};

struct ProgramArguments
{
    GraphicsType                  graphics_type;
    std::pair<uint32_t, uint32_t> window_size;
    std::size_t                   simulate;
    bool                          simulate_pve;
    bool                          simulate_pvp;
    std::uint32_t                 tick_time;
};

ProgramArguments GetProgramArguments( int argc, const char *argv[]);

} // ! namespace snake

#endif // ! SNAKE_ARGUMENTS_HH__
