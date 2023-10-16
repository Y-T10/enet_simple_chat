#include "textRenderer.hpp"

static_assert(bool(-1) == true);
static_assert(bool(0) == false);

const bool WriteText(SDL_Texture* texture, const Font& textFont, const SDL_Color& textColor, const std::string& text) noexcept {
    SDL_Rect textArea = {.x = 0, .y = 0};
    if(TTF_SizeUTF8(const_cast<TTF_Font*>(textFont->get()), text.c_str(), &textArea.w, &textArea.h)) {
        return false;
    }

    //SDL_Surface* textBuff = TTF_RenderUTF8_Blended(const_cast<TTF_Font*>(textFont->get()), text.c_str(), textColor);
    const SDL_Color colorBG = {.r = 255, .g = 255, .b = 255, .a = 0};
    SDL_Surface* textBuff = TTF_RenderUTF8_LCD(const_cast<TTF_Font*>(textFont->get()), text.c_str(), textColor, colorBG);
    if(!textBuff) {
        return false;
    }

    SDL_Surface* textureBuff = NULL;
    if(SDL_LockTextureToSurface(texture, NULL, &textureBuff)) {
        return false;
    }
    const int resutl = SDL_BlitSurface(textBuff, NULL, textureBuff, &textArea);
    SDL_UnlockTexture(texture);
    SDL_FreeSurface(textBuff);
    return !(bool(resutl));
}
