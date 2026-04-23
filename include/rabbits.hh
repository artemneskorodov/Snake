#ifndef SNAKE_RABBITS_HH__
#define SNAKE_RABBITS_HH__

namespace snake
{
namespace rabbits_detail
{

//
// kHeightParameter is the probability of rabbit spawn on target density
// kSharpnessParameter is just some sharpness parameter.
// Be careful while changing them, it can occur in negative probability in some cases.
//
// kRabbitsDensityTarget is target for rabbits density.
// kRabbitsMinDensity is minimal density of rabbits, probability equals 1 if density is less.
// kRabbitsMaxDensity is maximum density of rabbits, probability equals 0 if density is bigger.
//
// Change other parameters only if you know what you are doing.
//

constexpr float kSharpnessParameter = 10.f;

constexpr float    kRabbitsDensityTarget   = 0.02f;
constexpr float    kRabbitsMinDensity      = 0.00f;
constexpr float    kRabbitsMaxDensity      = 0.04f;
constexpr float    kRabbitsTargetSpawnProb = 0.1f;

constexpr inline float
map_min_max( float density)
{
    return (density - kRabbitsMinDensity) / (kRabbitsMaxDensity - kRabbitsMinDensity);
}

constexpr float x0 = map_min_max( kRabbitsTargetSpawnProb);
constexpr float y0 = kRabbitsTargetSpawnProb;

//
// f(x) = a1 x^5 + a2 x^4 + a3 x^3 + a4 x^2 + a5 x + a6 is a flat step.
// This formulas define coefficients for flat step. Do not change them unless you know what
// are you doing.
//
// This formulas are result of solving equation:
// f(0) = 1, f'(0) = 0, f(1) = 0, f'(1) = 0, f(x0) = y0
//
constexpr float a1 = kSharpnessParameter;
constexpr float tmp_1 = 2.f * x0 * x0 * x0 - 3.f * x0 * x0 + 1 - y0;
constexpr float tmp_2 = x0 * x0 + x0 - 2;
constexpr float a2 = -tmp_1 / (x0 * x0 * (x0 - 1.f) * (x0 - 1.f)) - a1 * tmp_2 / (x0 - 1);
constexpr float a3 = 2.f - 3.f * a1 - 2.f * a2;
constexpr float a4 = 2.f * a1 + a2 - 3.f;
constexpr float a5 = 0.f;
constexpr float a6 = 1.f;

constexpr inline float
get_probability( float x)
{
    return a1 * (x * x * x * x * x) +
           a2 * (x * x * x * x) +
           a3 * (x * x * x) +
           a4 * (x * x) +
           a5 * (x) +
           a6;
}

} // ! namespace detail

inline float
GetRabbitSpawnProbability( float density)
{
    if ( density < rabbits_detail::kRabbitsMinDensity )
    {
        return 1.f;
    } else if ( density > rabbits_detail::kRabbitsMaxDensity )
    {
        return 0.f;
    } else
    {
        float mapped_to_0_1 = rabbits_detail::map_min_max( density);
        return rabbits_detail::get_probability( mapped_to_0_1);
    }
}

} // ! namespace snake

#endif // ! SNAKE_RABBITS_HH__
