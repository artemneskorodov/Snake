#ifndef SNAKE_UTILS_HH__
#define SNAKE_UTILS_HH__

#include <random>

namespace snake
{
namespace utils
{

namespace random_detail
{

inline std::mt19937&
get_engine()
{
    static std::mt19937 engine{ std::random_device{}()};
    return engine;
}

} // ! namespace random_detail

namespace random
{

inline void
SetSeed( std::uint32_t seed)
{
    random_detail::get_engine().seed( seed);
}

inline void
SetRandomized()
{
    random_detail::get_engine().seed( std::random_device{}());
}

} // ! namespace random

template<typename NumberT>
NumberT
random_normal( NumberT mean,
               NumberT sigma)
{
    std::normal_distribution<double> distribution( mean, sigma);
    double result = distribution( random_detail::get_engine());
    return static_cast<NumberT>( std::round( result));
}

template<typename NumberT>
NumberT
random_min_max( NumberT min, NumberT max)
{
    std::uniform_int_distribution<NumberT> distribution( min, max);
    return distribution( random_detail::get_engine());
}

template<typename NumberT>
NumberT
random_of( std::initializer_list<NumberT> list)
{
    std::uniform_int_distribution<std::size_t> distribution( 0, list.size() - 1);
    std::size_t index = distribution( random_detail::get_engine());

    auto it = list.begin();
    for ( std::size_t i = 0; i != index; ++i )
    {
        ++it;
    }
    return *it;
}

inline bool
random_true_false( double p_of_true)
{
    std::bernoulli_distribution distribution( p_of_true);

    return distribution( random_detail::get_engine());
}

} // ! namespace utils
} // ! namespace snake

#endif // ! SNAKE_UTILS_HH__
