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

private:
    void render_snake( const Snake& snake);
    void render_rabbit( const Rabbit& rabbit);
private:
    sf::RenderWindow window;
    sf::Texture snake_texture_straight_;
    sf::Texture snake_texture_turning_;
};

} // ! namespace snake

#endif // ! SNAKE_GRAPHICS_VIEW_HH__
