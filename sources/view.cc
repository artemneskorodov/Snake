#include "view.hh"

namespace snake
{

void
View::update_common_events()
{
    auto old_window_size = current_window_size_;
    RequestWindowSize();
    if ( old_window_size != current_window_size_ )
    {
        events_.emplace_back( Event::WINDOW_SIZE_CHANGED);
    }
}

} // ! namespace snake
