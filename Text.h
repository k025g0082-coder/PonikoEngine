#pragma once

#include "wrl.h"

using namespace Microsoft::WRL;

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Sprite.h"

#include <string>
#include <map>

// フォントの設定
struct fontLayout {

	// 文字サイズ
	int textSize = 0;

	// 文字色
	Vector3 textColor{ 255.0f,255.0f,255.0f };

	// 縁取り
	int outlineSize = 0;

	// 縁取り色
	Vector3 outlineColor;

	// 縁をぼかすか
	bool isOutlineBlur = false;
};

class Text {
public:

	// 使うフォント名
	void Init();

	// フォント設定
	bool setFont(const std::string& fontName);
	

	// 文字サイズ変更
	void SetFontSize(int size);
	
	// 縁取りの大きさ設定
	void SetFontOutlineSize(int s) { textLayout.outlineSize = s; }

	// 文字の色設定
	void SetFontColor(Vector3 c) {
		textLayout.textColor = c;
	}

	// 縁取りの色設定
	void SetFontOutlineColor(Vector3 c) { 
		textLayout.outlineColor = c;
	}

	// 複数の設定を纏めたレイアウトを設定
	void SetFontLayout(fontLayout l) {

		textLayout = l;

		SetFontSize(l.textSize);
	}

	// 文字のスタイル
	enum class TextAnchor {
		LeftTop,
		Center,
	};

	void drawString(const std::string& text, Vector2 position, TextAnchor anchor = TextAnchor::LeftTop);

	unsigned char* getImage();

	// 文字横幅取得
	int GetTextWidth(const std::string& text);
	int GetTextHeight(const std::string& text);

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

	// 描画開始位置などを設定
	void CursorSet();

	void draw_bitmap_outline(int x, int y);
	void draw_bitmap_core(int x, int y,int r,int g, int b, unsigned char alphaScale = 255);

	std::string  str;

	FontData* font = nullptr;

	// 描画用
	Sprite sprite;

	int width;
	int height;
	
	// 縁取りの色とか纏めた
	fontLayout textLayout;


	std::u32string u32text_;
	Vector2 position_;
	TextAnchor anchor_;

	int curPosX = 0;
	int curPosY = 0;

	int last_height = 0;
	int fontHeight = 0;
	int textWidth = 0;

};



std::u32string ToU32String(const std::string& str);
