#ifndef SNAKE_RABBITS_HH__
#define SNAKE_RABBITS_HH__

namespace snake
{

namespace rabbits_detail
{

constexpr float kRabbitsMinDensity   = 0.00f;
constexpr float kRabbitsMaxDensity   = 0.03f;
constexpr float kRabbitsMaxSpawnProb = 0.2f;

//
// Maps [kRabbitsMinDensity, kRabbitsMaxDensity] interval to [0, 1]
//

constexpr inline float
map_min_max( float density)
{
    return (density - kRabbitsMinDensity) / (kRabbitsMaxDensity - kRabbitsMinDensity);
}

///
/// @brief Smooth step which equals to 1 in x=0 and 0 in x=1
///
/// @param x value in range [0; 1]
///
inline float
smooth_step( float x)
{
    constexpr float a1 =  1.0f;
    constexpr float a2 = -2.5f;
    constexpr float a3 =  4.0f;
    constexpr float a4 = -3.5f;
    constexpr float a5 =  0.0f;
    constexpr float a6 =  1.0f;
    return a1 * std::pow( x, 5.f) +
           a2 * std::pow( x, 4.f) +
           a3 * std::pow( x, 3.f) +
           a4 * std::pow( x, 2.f) +
           a5 * std::pow( x, 1.f) +
           a6 * std::pow( x, 0.f);
}

} // ! namespace detail

inline float
GetRabbitSpawnProbability( float density)
{
    if ( density < rabbits_detail::kRabbitsMinDensity )
    {
        return rabbits_detail::kRabbitsMaxSpawnProb;
    } else if ( density > rabbits_detail::kRabbitsMaxDensity )
    {
        return 0.f;
    } else
    {
        float mapped_to_0_1 = rabbits_detail::map_min_max( density);
        return rabbits_detail::kRabbitsMaxSpawnProb *
               rabbits_detail::smooth_step( mapped_to_0_1);
    }
}

} // ! namespace snake

#endif // ! SNAKE_RABBITS_HH__
