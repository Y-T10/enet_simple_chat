#include "font.hpp"

#include "fontconfig/fontconfig.h"

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

const Font GetFontPaht(FcPattern* fontPat) noexcept {
	if (FcChar8* file = NULL; FcPatternGetString(fontPat, FC_FILE, 0, &file) == FcResultMatch) {
		SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "found font path: %s\n", (char*)file);
        return Font(new Font_impl((char*)file));
	}
    return nullptr;
}

const Font LoadFontInternal(FcConfig* config, FcPattern* pat) noexcept {
    FcResult result;
    FcPattern* font = FcFontMatch(config, pat, &result);
    if(!font) {
        return nullptr;
    }
    const Font foundFont = (result == FcResult::FcResultMatch)?
                                GetFontPaht(font):
                                nullptr;
    FcPatternDestroy(font);
    return foundFont;
}

const Font LoadFont(const std::string& fontName) {
	FcInit();
	FcConfig* config = FcInitLoadConfigAndFonts();

	FcPattern* pat = FcNameParse((const FcChar8*)fontName.c_str());
	FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
    const auto foundFont = LoadFontInternal(config, pat);
	FcPatternDestroy(pat);
	FcConfigDestroy(config);
	FcFini();

    return foundFont;
}

/*
int main() {
    FcInit(); // Fontconfigの初期化

    FcPattern *pattern = FcPatternCreate(); // パターンを作成
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern); // パターンに設定を適用
    FcDefaultSubstitute(pattern); // パターンにデフォルト値を適用

    const char *fontName = "Arial"; // 検索したいフォント名

    FcPattern *matchPattern = FcNameParse((const FcChar8 *)fontName); // 検索するフォント名のパターンを作成

    FcObjectSet *os = FcObjectSetBuild(FC_FILE, FC_FAMILY, FC_STYLE, (const FcChar8 *)nullptr); // 戻り値に必要なプロパティを指定

    FcFontSet *fontSet = FcFontList(nullptr, pattern, os); // フォント一覧を取得

    if (fontSet) {
        for (int i = 0; i < fontSet->nfont; ++i) {
            FcPattern *fontPattern = fontSet->fonts[i];
            FcChar8 *fontFile;
            if (FcPatternGetString(fontPattern, FC_FILE, 0, &fontFile) == FcResultMatch) {
                std::cout << "Font File: " << fontFile << std::endl;
            }
        }

        FcFontSetDestroy(fontSet);
    }

    FcPatternDestroy(pattern);
    FcPatternDestroy(matchPattern);
    FcObjectSetDestroy(os);

    FcFini(); // Fontconfigの終了

    return 0;
}
 */