#ifndef SNAKE_GRAPHICS_VIEW_HH__
#define SNAKE_GRAPHICS_VIEW_HH__

#include <SFML/Graphics.hpp>

#include "view.hh"

namespace snake
{

struct Textures
{
    sf::Texture snake_texture_body_straight { "resources/snake_texture_body_straight.png"};
    sf::Texture snake_texture_body_turning  { "resources/snake_texture_body_turning.png"};
    sf::Texture snake_texture_head_straight { "resources/snake_texture_head_straight.png"};
    sf::Texture snake_texture_head_turning  { "resources/snake_texture_head_turning.png"};
    sf::Texture snake_texture_tail          { "resources/snake_texture_tail.png"};
};

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
    sf::RenderWindow window_;
    Textures         textures_;

};

} // ! namespace snake

#endif // ! SNAKE_GRAPHICS_VIEW_HH__
