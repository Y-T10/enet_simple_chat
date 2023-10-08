#pragma once

#include "SDL_timer.h"
#include "SDL_render.h"
#include "utility.hpp"
#include "SDL2/SDL.h"
#include <cstdint>
#include <tuple>

class SceneTitle final {
    public:
    template <class Parameter>
    void operator()(const Parameter& param) {
        static_assert(check_tuple_v<Parameter, SDL_Window*>);
        static_assert(check_tuple_v<Parameter, SDL_Renderer*>);

        SDL_Rect mouseposrect;
        Uint8 r;
        SDL_bool keep_going = SDL_TRUE;
        SDL_Event event;
        SDL_Renderer* renderer = std::get<SDL_Renderer*>(param);

        mouseposrect.x = mouseposrect.y = -1000;  /* -1000 so it's offscreen at start */
        mouseposrect.w = mouseposrect.h = 50;

        /* run the program until told to stop. */
        while (keep_going) {

            /* run through all pending events until we run out. */
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:  /* triggers on last window close and other things. End the program. */
                        keep_going = SDL_FALSE;
                        break;

                    case SDL_KEYDOWN:  /* quit if user hits ESC key */
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            keep_going = SDL_FALSE;
                        }
                        break;

                    case SDL_MOUSEMOTION:  /* keep track of the latest mouse position */
                        /* center the square where the mouse is */
                        mouseposrect.x = event.motion.x - (mouseposrect.w / 2);
                        mouseposrect.y = event.motion.y - (mouseposrect.h / 2);
                        break;
                }
            }

            /* fade between shades of red every 3 seconds, from 0 to 255. */
            r = (Uint8) ((((float) (SDL_GetTicks() % 3000)) / 3000.0f) * 255.0f);
            SDL_SetRenderDrawColor(renderer, r, 0, 0, 255);

            /* you have to draw the whole window every frame. Clearing it makes sure the whole thing is sane. */
            SDL_RenderClear(renderer);  /* clear whole window to that fade color. */

            /* set the color to white */
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            /* draw a square where the mouse cursor currently is. */
            SDL_RenderFillRect(renderer, &mouseposrect);

            /* put everything we drew to the screen. */
            SDL_RenderPresent(renderer);
        }
    }
};