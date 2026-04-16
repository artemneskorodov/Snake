#ifndef SNAKE_GRAPHICS_VIEW_HH__
#define SNAKE_GRAPHICS_VIEW_HH__

#include <SFML/Graphics.hpp>

#include "view.hh"

namespace snake
{

class GraphicsView : public View
{
public:
    GraphicsView( uint32_t width, uint32_t height);
    ~GraphicsView();

public:
    void                              Render( const Model& model)       override;
    std::pair<Coordinate, Coordinate> GetGameFieldSize()          const override;
    void                              UpdateEvents()                    override;
    void                              RequestWindowSize()               override;

private:
    sf::RenderWindow window;
};

} // ! namespace snake

#endif // ! SNAKE_GRAPHICS_VIEW_HH__
