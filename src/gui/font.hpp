#pragma once

#include "SDL_ttf.h"
#include <filesystem>
#include "boost/smart_ptr/intrusive_ref_counter.hpp"
#include "boost/smart_ptr/intrusive_ptr.hpp"

class Font_impl : public boost::intrusive_ref_counter<Font_impl> {
    public:
    Font_impl(const std::filesystem::path& ttfPath) noexcept;
    ~Font_impl() noexcept;

    Font_impl(const Font_impl& rval) noexcept = delete;
    Font_impl& operator=(const Font_impl& rval) noexcept = delete;
    
    Font_impl(Font_impl&& rval) noexcept;
    Font_impl& operator=(Font_impl&& rval) noexcept;

    operator bool() noexcept;

    void setFontSize(const int pt) noexcept;
    const TTF_Font* get() noexcept;

    private:
    TTF_Font *m_font;
};

using Font = boost::intrusive_ptr<Font_impl>;