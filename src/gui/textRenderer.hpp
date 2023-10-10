#pragma once

#include "SDL_ttf.h"
#include <filesystem>
#include "boost/smart_ptr/intrusive_ref_counter.hpp"
#include "boost/smart_ptr/intrusive_ptr.hpp"

class Font_impl : public boost::intrusive_ref_counter<Font_impl> {
    public:
    Font_impl(const std::filesystem::path& ttfPath) noexcept:
    m_font(TTF_OpenFont(ttfPath.c_str(), 12)){ };

    ~Font_impl() noexcept {
        TTF_CloseFont(m_font);
        m_font = NULL;
    }

    void setFontSize(const int pt) noexcept {
        TTF_SetFontSize(m_font, pt);
    };

    const TTF_Font* getFontRaw() noexcept {
        return m_font;
    };

    operator bool() noexcept {
        return m_font != NULL;
    }

    private:
    TTF_Font *m_font;
};

using Font = boost::intrusive_ptr<Font_impl>;
