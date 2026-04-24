#ifndef SNAKE_GRAPHICS_VIEW_HH__
#define SNAKE_GRAPHICS_VIEW_HH__

#include <SFML/Graphics.hpp>

#include "view.hh"
#include "game_settings.hh"

namespace snake
{

struct Textures
{
    sf::Texture snake_texture_body_straight { "resources/snake_texture_body_straight.png"};
    sf::Texture snake_texture_body_turning  { "resources/snake_texture_body_turning.png"};
    sf::Texture snake_texture_head_straight { "resources/snake_texture_head_straight.png"};
    sf::Texture snake_texture_head_turning  { "resources/snake_texture_head_turning.png"};
    sf::Texture snake_texture_tail          { "resources/snake_texture_tail.png"};
    sf::Texture snake_bone_texture          { "resources/snake_bone_texture.png"};
    sf::Texture rabbit_texture              { "resources/rabbit_texture.png"};

    sf::Font    snake_game_font             { "resources/HennyPenny-Regular.ttf"};
    sf::Font    snake_game_text_font        { "resources/OpenSans-VariableFont_wdth,wght.ttf"};

};

class GraphicsView : public View
{
public:
    GraphicsView( uint32_t width, uint32_t height);
    ~GraphicsView();

public:
    void                              Render( const Model& model)                       override;
    void                              RenderMenu( const settings::Menu& settings)       override;
    std::pair<Coordinate, Coordinate> GetGameFieldSize()                          const override;
    void                              UpdateEvents()                                    override;
    void                              UpdateMenuEvents()                                override;
    ViewUpdateCallbacks               GetCallbacks()                              const override;
    void                              RenderAll( const Model& model)                    override;
    void                              Show()                                            override;

private:
    void render_snake( const Snake& snake);
    void render_rabbit( const Rabbit& rabbit);
    void render_bone( const Bone& bone);
    void render_game_field( const Model& model);
    void render_menu_button( const settings::MenuElement& menu_elem, float& offset_y);
    void render_menu_snakes_list( const settings::MenuElement& menu_elem, float& offset_y);
    void render_game_statistics( const Model& model);
    float draw_group_stats( const SnakeGroupStatistics& stats,
                            float                       x,
                            float                       y,
                            const std::string&          name);
    void draw_snake_stats( const Snake& snake, float x, float y, SnakeGroup group);

private:
    constexpr sf::Vector2f game_to_sfml( Coordinate x, Coordinate y);
    constexpr sf::Vector2f game_to_sfml( const Point& point);

private:
    sf::RenderWindow window_;
    Textures         textures_;

};

} // ! namespace snake

#endif // ! SNAKE_GRAPHICS_VIEW_HH__
