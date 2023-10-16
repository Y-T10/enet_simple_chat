#include "font.hpp"

Font_impl::Font_impl(const std::filesystem::path& ttfPath) noexcept:
m_font(TTF_OpenFont(ttfPath.c_str(), 12)){ };

Font_impl::~Font_impl() noexcept {
    TTF_CloseFont(m_font);
    m_font = NULL;
}
    
Font_impl::Font_impl(Font_impl&& rval) noexcept:
m_font(std::move(rval.m_font)){
    rval.m_font = NULL;
};

Font_impl& Font_impl::operator=(Font_impl&& rval) noexcept {
    if(this == &rval){ 
        return *this;
    }
    m_font = std::move(rval.m_font);
    rval.m_font = NULL;
    return *this;
};

void Font_impl::setFontSize(const int pt) noexcept {
    TTF_SetFontSize(m_font, pt);
};

const TTF_Font* Font_impl::get() noexcept {
    return m_font;
};

Font_impl::operator bool() noexcept {
    return m_font != NULL;
}