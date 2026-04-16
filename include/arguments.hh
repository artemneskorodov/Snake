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
    GraphicsType graphics_type;
    std::pair<uint32_t, uint32_t> window_size; // Used if only graphics type is GraphicsType::SFML
    std::size_t humans;
    std::size_t dumb_bots;
    std::size_t smart_bots;
};

ProgramArguments GetProgramArguments( int argc, const char *argv[]);

} // ! namespace snake

#endif // ! SNAKE_ARGUMENTS_HH__
