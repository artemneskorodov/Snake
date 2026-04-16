#ifndef SNAKE_BOTS_HH__
#define SNAKE_BOTS_HH__

#include "model.hh"

namespace snake
{
namespace bots
{

void TickDumbBot( Model& model, const Snake& snake);
void TickSmartBot( Model& model, const Snake& snake);

} // ! namespace bots
} // ! namespace snake

#endif // ! SNAKE_BOTS_HH__
