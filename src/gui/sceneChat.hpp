#pragma once

#include "SDL_blendmode.h"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_timer.h"
#include "SDL_render.h"
#include "utility.hpp"
#include "SDL2/SDL.h"

#include "textRenderer.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <tuple>

const SDL_Rect GetTextureSize (const SDL_Texture* texture) noexcept {
    SDL_Rect size = {.x = 0 , .y = 0};
    SDL_QueryTexture(const_cast<SDL_Texture*>(texture), NULL, NULL, &size.w, &size.h);
    return size;
}

class SceneChat final {
    public:
    template <class Parameter>
    void operator()(const Parameter& param) {
        static_assert(check_tuple_v<Parameter, SDL_Window*>);
        static_assert(check_tuple_v<Parameter, SDL_Renderer*>);
        
        SDL_Renderer* renderer = std::get<SDL_Renderer*>(param);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
        
        SDL_Texture* textBuff = SDL_CreateTexture(renderer,
            SDL_PixelFormatEnum::SDL_PIXELFORMAT_ARGB8888,
            SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING,
            800, 600
        );

        if(!textBuff){
            return;
        }
        SDL_SetTextureBlendMode(textBuff, SDL_BlendMode::SDL_BLENDMODE_BLEND);

        const SDL_Rect textBuffSize = GetTextureSize(textBuff);

        //auto jp_font = LoadFont("", );

        Font jp_font(new Font_impl("/usr/share/fonts/truetype/fonts-japanese-gothic.ttf"));
        jp_font->setFontSize(18);
        assert(jp_font);

        std::string inputText = "";

        if(SDL_IsTextInputActive()){
            SDL_StopTextInput();
        }

        /* run the program until told to stop. */
        for (SDL_bool keep_going = SDL_TRUE; keep_going;) {
            /* run through all pending events until we run out. */
            for (SDL_Event event;SDL_PollEvent(&event);) {
                switch (event.type) {
                    case SDL_QUIT:  /* triggers on last window close and other things. End the program. */
                        keep_going = SDL_FALSE;
                        break;

                    case SDL_KEYDOWN:  /* quit if user hits ESC key */
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            keep_going = SDL_FALSE;
                            break;
                        }
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            const SDL_Rect textBuffSize = {.x = 0, .y = 0, .w = 100, .h = 100};
                            SDL_SetTextInputRect(&textBuffSize);
                            SDL_StartTextInput();
                            break;
                        }
                        break;
                    case SDL_TEXTINPUT:
                        inputText = std::string(event.text.text);
                        if(SDL_IsTextInputActive()){
                            SDL_StopTextInput();
                        }
                        SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "input: %s", inputText.c_str());
                        break;
                    case SDL_TEXTEDITING:
                        inputText = std::string(event.edit.text);
                        break;
                    case SDL_TEXTEDITING_EXT:
                        inputText = std::string(event.editExt.text);
                        SDL_free(event.editExt.text);
                        break;
                }
            }

            /* set the color to white */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            /* you have to draw the whole window every frame. Clearing it makes sure the whole thing is sane. */
            SDL_RenderClear(renderer);  /* clear whole window to that fade color. */

            if(!WriteText(textBuff, jp_font, SDL_Color{.r = 0, .g = 100, .b = 150, .a= 255}, inputText)){
                fprintf(stderr, "%s\n", SDL_GetError());
                SDL_DestroyTexture(textBuff);
                return;
            }

            SDL_RenderCopy(renderer, textBuff, NULL, &textBuffSize);

            /* put everything we drew to the screen. */
            SDL_RenderPresent(renderer);
        }

        SDL_DestroyTexture(textBuff);
    }
};