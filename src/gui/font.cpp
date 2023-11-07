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
	FcChar8* file = NULL;
    // パターンからフォントファイルへのパスを得る
    if (FcPatternGetString(fontPat, FC_FILE, 0, &file) != FcResultMatch) {
        return nullptr;
    }

	SDL_LogInfo(SDL_LOG_CATEGORY_TEST, "found font path: %s\n", (char*)file);
    const auto filePaht = std::filesystem::path((char*)file);
    if(filePaht.empty() || !std::filesystem::exists(filePaht)) {
        return nullptr;
    }
    return MakeFont(filePaht);
}

const Font LoadFontInternal(FcConfig* config, FcPattern* pat) noexcept {
    FcResult result;
    // configに従ってpatのパターンに最も近いフォントの情報を返す．
    // 検索結果はresultが持ち、フォントの情報は関数が返す．
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

// 書体: 統一のデザインで表現された字形の集合．スタイルまでを気にしてフォントを指す際に使用されることが多い(主観では)．
// フォントファミリー: 複数の書体を束ねるグループ．フォントの名前として使用されることが多い(主観では)．
const Font LoadFont(const std::string& fontName) {
	FcInit();
    // 既定のfontconfig設定ファイルの読み込む．
    // 既定の利用可能なフォントに関する情報を構築する．
	FcConfig* config = FcInitLoadConfigAndFonts();
    // fontconfig設定は、パターンの修正とフォントに関する情報を持つ．
    // コレに情報を追加することで、独自のフォントデータベースが構築できる．

    // フォント名からパターンを生成する
    // TODO: パースではなく、オブジェクト構築に変更する
	FcPattern* pat = FcNameParse((const FcChar8*)fontName.c_str());
    // fontconfig設定に従ってパターンを修正する．
	FcConfigSubstitute(config, pat, FcMatchPattern);
    // 既定の置き換えによって修正したパターンを正規化する．
    // 正規化することで、後の検索処理を容易にする．
	FcDefaultSubstitute(pat);
    const auto foundFont = LoadFontInternal(config, pat);
	FcPatternDestroy(pat);
	FcConfigDestroy(config);
	FcFini();

    return foundFont;
}

/*
参考文献
- https://www.freedesktop.org/software/fontconfig/fontconfig-user.html
- https://gist.github.com/CallumDev/7c66b3f9cf7a876ef75f
- https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
- https://www.freedesktop.org/software/fontconfig/fontconfig-devel/x103.html#AEN106
*/