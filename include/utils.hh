#ifndef SNAKE_UTILS_HH__
#define SNAKE_UTILS_HH__

#include <random>

namespace snake
{
namespace utils
{

template<typename NumberT>
NumberT
random_normal( NumberT mean,
               NumberT sigma)
{
    std::random_device rd;
    std::default_random_engine gen( rd());

    std::normal_distribution<double> distribution( mean, sigma);
    double result = distribution( gen);
    return static_cast<NumberT>( std::round( result));
}

template<typename NumberT>
NumberT
random_min_max( NumberT min, NumberT max)
{
    std::random_device rd;
    std::default_random_engine gen( rd());

    std::uniform_int_distribution<NumberT> distribution( min, max);
    return distribution( gen);
}

} // ! namespace utils
} // ! namespace snake

#endif // ! SNAKE_UTILS_HH__
