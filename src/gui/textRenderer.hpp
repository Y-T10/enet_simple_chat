#pragma once

#include "SDL_ttf.h"
#include <filesystem>
#include <string>

#include "SDL_pixels.h"
#include "SDL_render.h"

#include "font.hpp"

/// 内部でフォントのキャッシュを持つ
class FontDB {
    public:
};

/// TTFのSurfaceをTextureのSurfaceに返す．
/// 将来的にはTextureに直接書き込めるようにする．
const bool WriteText(SDL_Texture* texture, const Font& textFont, const SDL_Color& textColor,  const std::string& text) noexcept;
