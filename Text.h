#pragma once

#include "wrl.h"

using namespace Microsoft::WRL;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Sprite.h"

#include <string>
#include <map>



class Text {
public:

	// 使うフォント名
	void Init();

	// フォント設定
	bool setFont(const std::string& fontName);

	// 文字サイズ変更
	void SetFontSize(int size);

	// 文字のスタイル
	enum class TextAnchor {
		LeftTop,
		Center,
	};

	void drawString(const std::u32string& text, Vector2 position, TextAnchor anchor = TextAnchor::LeftTop);

	unsigned char* getImage();

	// 文字システム初期化
	static void Initialize();

	// 新規フォント登録
	static bool RegisterFont(std::string fontName, const char* font_file_name, int char_width, int char_height);

private:

	static FT_Library library;   /* handle to library     */

	struct FontData {
		FT_Face face = nullptr;
		FT_GlyphSlot slot = nullptr;  // グリフへのショートカット
	};
	static std::map<std::string, FontData> fonts;  /* フォントたち */

	unsigned char* image;

	void draw_bitmap(int x, int y);

	std::string  str;

	FontData* font = nullptr;

	// 描画用
	Sprite sprite;

	int width;
	int height;
};

