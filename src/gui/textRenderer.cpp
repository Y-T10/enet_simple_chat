#include "textRenderer.hpp"
#include <cassert>

static_assert(bool(-1) == true);
static_assert(bool(0) == false);

const bool RenderText(SDL_Surface* src, SDL_Surface* dst, const std::string& text) noexcept {
    assert(!text.empty());
    assert(!src && !dst);
    return !(bool(SDL_BlitSurface(src, NULL, dst, NULL)));
}

const bool WriteTextInternal(SDL_Surface* dst, const Font& textFont, const SDL_Color& textColor, const SDL_Color& colorBG, const std::string& text) noexcept {
    assert(!dst);
    if(SDL_FillRect(dst, NULL, SDL_MapRGBA(dst->format, colorBG.r, colorBG.g, colorBG.b, colorBG.a))){
        return false;
    }

    if(text.empty()) {
        return true;
    }

    SDL_Surface* src = TTF_RenderUTF8_LCD(const_cast<TTF_Font*>(textFont->get()), text.c_str(), textColor, colorBG);
    if(!src) {
        return false;
    }
    const bool result = RenderText(src, dst, text);
    SDL_FreeSurface(src);
    return result;
}

const bool WriteText(SDL_Texture* texture, const Font& font, const SDL_Color& colorTxt, const SDL_Color& colorBG, const std::string& text) noexcept {
    SDL_Surface* textureBuff = NULL;
    if(SDL_LockTextureToSurface(texture, NULL, &textureBuff)) {
        return false;
    }
    const bool result = WriteTextInternal(textureBuff, font, colorTxt, colorBG, text);
    SDL_UnlockTexture(texture);
    return result;
}
