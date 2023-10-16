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

    Font_impl(const Font_impl& rval) noexcept = delete;
    Font_impl& operator=(const Font_impl& rval) noexcept = delete;
    
    Font_impl(Font_impl&& rval) noexcept:
    m_font(std::move(rval.m_font)){
        rval.m_font = NULL;
    };

    Font_impl& operator=(Font_impl&& rval) noexcept {
        if(this == &rval){
            return *this;
        }

        m_font = std::move(rval.m_font);
        rval.m_font = NULL;

        return *this;
    }

    void setFontSize(const int pt) noexcept {
        TTF_SetFontSize(m_font, pt);
    };

    const TTF_Font* get() noexcept {
        return m_font;
    };

    operator bool() noexcept {
        return m_font != NULL;
    }

    private:
    TTF_Font *m_font;
};

using Font = boost::intrusive_ptr<Font_impl>;

/// 内部でフォントのキャッシュを持つ
class FontDB {
    public:
};
