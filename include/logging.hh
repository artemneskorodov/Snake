#ifndef SNAKE_LOGGING_HH__
#define SNAKE_LOGGING_HH__

#include <string_view>
#include <fstream>
#include <cstdarg>

namespace snake
{

#define SNAKE_DEBUG_ALL

static const std::string kDebugFileName = "logs/logs.txt";

template<typename... Args>
void
SnakeDebugMsg(std::string_view level, Args... args)
{
    // FIXME open file as static variable, or do something like this to avoid reopening
    std::ofstream out{ kDebugFileName, std::ios::app};
    if ( !out )
    {
        throw std::runtime_error{ "Log file not opened: " +
                                  std::string{ std::strerror( errno)}};
    }

    out << level;

    (out << ... << args) << std::endl;
}

#ifdef SNAKE_DEBUG_ALL

    #define DEBUG_INFO(...)    SnakeDebugMsg(    "[INFO] ", __VA_ARGS__)
    #define DEBUG_WARNING(...) SnakeDebugMsg( "[WARNING] ", __VA_ARGS__)
    #define DEBUG_ERROR(...)   SnakeDebugMsg(   "[ERROR] ", __VA_ARGS__)

#else // SNAKE_DEBUG_ALL

    #define DEBUG_INFO(...)    do {} while(0)
    #define DEBUG_WARNING(...) do {} while(0)
    #define DEBUG_ERROR(...)   do {} while(0)

#endif // SNAKE_DEBUG_ALL

} // ! namespace snake

#endif // ! SNAKE_LOGGING_HH__
