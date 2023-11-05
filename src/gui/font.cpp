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

const Font LoadFont(const std::string& fontName) {
    // Fontconfigの初期化
    if (!FcInit()) {
        return nullptr;
    }

    FcPattern *pattern = FcPatternCreate(); // パターンを作成
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern); // パターンに設定を適用
    FcDefaultSubstitute(pattern); // パターンにデフォルト値を適用

    FcPattern *matchPattern = FcNameParse((const FcChar8 *)fontName.c_str()); // 検索するフォント名のパターンを作成

    FcObjectSet *os = FcObjectSetBuild(FC_FILE, FC_FAMILY, FC_STYLE, (const FcChar8 *)nullptr); // 戻り値に必要なプロパティを指定

    FcFontSet *fontSet = FcFontList(nullptr, pattern, os); // フォント一覧を取得

    // TODO: このループをサブ関数に分割する
    FcChar8 *fontFile = nullptr;
    if (fontSet) {
        for (int i = 0; i < fontSet->nfont; ++i) {
            FcPattern *fontPattern = fontSet->fonts[i];
            if (FcPatternGetString(fontPattern, FC_FILE, 0, &fontFile) == FcResultMatch) {
                break;
            }
        }

        FcFontSetDestroy(fontSet);
    }

    // TODO: パスがfilesystem::pathで有効かを事前に調べる
    const Font font = new Font_impl((char*)(fontFile));

    FcPatternDestroy(pattern);
    FcPatternDestroy(matchPattern);
    FcObjectSetDestroy(os);

    FcFini(); // Fontconfigの終了

    return font;
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