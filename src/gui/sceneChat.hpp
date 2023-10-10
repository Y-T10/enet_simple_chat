#pragma once

#include "SDL_timer.h"
#include "SDL_render.h"
#include "utility.hpp"
#include "SDL2/SDL.h"
#include <cstdint>
#include <tuple>

class SceneChat final {
    public:
    template <class Parameter>
    void operator()(const Parameter& param) {
        static_assert(check_tuple_v<Parameter, SDL_Window*>);
        static_assert(check_tuple_v<Parameter, SDL_Renderer*>);

        //fontDB = createFontDB("dir/to/font", "dir/to/system/font", ...);
        //font = fontDB.getFont("Noto sans cjk.ttf");
        //font.setFontSize(10);
        //textdrawer.setTarget(renderer);
        //textdrawer.setFont(font);
        //textdrawer.setStartPos(0, 0);
        //WriteText(textdrawer, "Hello world.\nHello SDL2.");
    }
};