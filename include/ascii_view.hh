#ifndef SNAKE_ASCII_VIEW_HH__
#define SNAKE_ASCII_VIEW_HH__

#include <termios.h>
#include <array>

#include "view.hh"

namespace snake
{

class AsciiView final : public View
{
public:
    AsciiView();
    ~AsciiView();

public:
    void                              Render( const Model& model)       override;
    std::pair<Coordinate, Coordinate> GetGameFieldSize()          const override;
    void                              UpdateEvents()                    override;
    void                              RequestWindowSize()               override;

private:
    void render_snake( const Snake& snake);
    void render_rabbit( const Rabbit& rabbit);

public:
    static void clear_screen();
    static void go_to_xy( Coordinate x, Coordinate y);
    static void set_color( uint32_t rgb);
    static void draw_box( Coordinate  x,
                          Coordinate  y,
                          Coordinate  width,
                          Coordinate  height,
                          const char *top_left,
                          const char *left,
                          const char *bottom_left,
                          const char *bottom,
                          const char *bottom_right,
                          const char *right,
                          const char *top_right,
                          const char *top);
    static void draw_line( Coordinate  x1,
                           Coordinate  y1,
                           Coordinate  x2,
                           Coordinate  y2,
                           const char *symbol);

private:
    termios console_attr_saved_;

};

} // ! namespace snake

#endif // ! SNAKE_ASCII_VIEW_HH__
