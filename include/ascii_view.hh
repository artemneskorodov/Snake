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
    void render_bone( const Bone& bone);
    void render_game_statistics( const Model& model);

private:
    struct menu_render_ctx_t
    {
        Coordinate offset_y;
    };

    void render_menu_button( const settings::MenuElement& button, menu_render_ctx_t& ctx);
    void render_menu_snakes_list( const settings::MenuElement& snakes_list, menu_render_ctx_t& ctx);

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
    static void draw_box( Coordinate x, Coordinate y, Coordinate width, Coordinate height);
    static Coordinate draw_group_stats( const SnakeGroupStatistics& stats,
                                        Coordinate                  x,
                                        Coordinate                  y,
                                        const std::string&          name);
    static void render_snake_status( const Snake& snake, Coordinate x, Coordinate y, SnakeGroup group);

private:
    termios console_attr_saved_;

};

} // ! namespace snake

#endif // ! SNAKE_ASCII_VIEW_HH__
