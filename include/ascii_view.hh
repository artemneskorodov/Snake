#ifndef SNAKE_ASCII_VIEW_HH__
#define SNAKE_ASCII_VIEW_HH__

#include <termios.h>
#include <array>

#include "view.hh"
#include "colors.hh"
#include "game_settings.hh"

namespace snake
{

class AsciiView final : public View
{
public:
    AsciiView();
    ~AsciiView();

public:
    void                              Render( const Model& model)                       override;
    void                              RenderMenu( const settings::Menu& settings)       override;
    std::pair<Coordinate, Coordinate> GetGameFieldSize()                          const override;
    void                              UpdateEvents()                                    override;
    void                              UpdateMenuEvents()                                override;

private:
    void render_snake( const Snake& snake);
    void render_rabbit( const Rabbit& rabbit);
    void draw_game_box();
    void render_snake_status( const Snake& snake, Coordinate status_offset);
    void render_bone( const Bone& bone);

public:
    static void clear_screen();
    static void go_to_xy( Coordinate x, Coordinate y);
    static void set_color( const colors::Color& color, bool bold = false);
    static void draw_line( Coordinate  x1,
                           Coordinate  y1,
                           Coordinate  x2,
                           Coordinate  y2,
                           const char *symbol);
    static std::pair<Coordinate, Coordinate> get_window_size();

private:
    termios console_attr_saved_;

};

} // ! namespace snake

#endif // ! SNAKE_ASCII_VIEW_HH__
