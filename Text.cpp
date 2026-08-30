
#include "Text.h"

#include "WinApp.h"
#include "DirectXUtility.h"
#include "DirectXCommon.h"

#include <Windows.h>

#define NOMINMAX
#include <algorithm>

std::map<std::string, Text::FontData> Text::fonts;
FT_Library Text::library = nullptr;

void Text::Init() {

	// 描画する区域
	width = 1280; // 超長くできるように
	height = 300; // 高さは固定 dekai mojiha kokowo ookiku

	// 初期化
	delete[] image;
	image = nullptr;

	// キャンバス生成
	image = new unsigned char[width * height * 4]();

	// イメージのキャンバスのメモリを先に確保？？
	memset(image, 0, width * height * 4);

	// テキストテキスチャ用の初期化
	sprite.TextTextureInitialize(image, width, height);
}

// フォント設定
bool Text::setFont(const std::string& fontName)
{
	// 検索
	auto it = fonts.find(fontName);

	if (it == fonts.end())
	{
		DirectXUtility::Log("[ERROR] Font not found");
		return false;
	}

	font = &it->second;

	return true;
}

void Text::Initialize() {

	FT_Error error = FT_Init_FreeType(&library);

	if (error) {
		DirectXUtility::Log("[ERROR] FreeType initialize failed!");
		return;
	}
};

bool Text::RegisterFont(std::string fontName, const char* font_file_name, int char_width, int char_height) {

	// フォント設定
	FontData state{};

	//  faceを作成
	auto error = FT_New_Face(library, font_file_name, 0, &state.face);

	if (error == FT_Err_Unknown_File_Format) {
		DirectXUtility::Log("[ERROR] Font file format is not supported!! ");
		return false;
	}
	else if (error) {
		DirectXUtility::Log("[ERROR] Font file not found or it is broken! ");
		return false;
	}

	// 3. 文字サイズを設定
	error = FT_Set_Char_Size(state.face, 0,
		char_width * char_height, // 幅と高さ
		300, 300);  // 水平、垂直解像度*/

	state.slot = state.face->glyph;  // グリフへのショートカット

	// 登録
	fonts[fontName] = state;

	return !error;
}


void Text::SetFontSize(int size)
{
	if (!font)
		return;

	FT_Set_Char_Size(
		font->face,
		0,
		size * 64,
		300,
		300
	);
}


// 文字列描画
void Text::drawString(const std::string& text, Vector2 position, TextAnchor anchor)
{

	std::u32string u32text = ToU32String(text);

	// キャンバス初期化
	memset(image, 0, width * height * 4);

	// だいにゅ０－
	anchor_ = anchor;
	position_ = position;
	u32text_ = u32text;

	textWidth = 0;

	// 全体の横幅取得
	for (char32_t c : u32text_)
	{
		if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
			continue;

		textWidth += font->slot->advance.x >> 6;
	}


	// 描画位置とかを設定
	CursorSet();


	// 1) 縁取りを描画
	// 2) 本体の文字を描画

	// 1.縁取りを先に描画
	for (char32_t c : u32text_)
	{
		if (c == U'\n')
		{
			curPosX = static_cast<int>(position.x);
			curPosY += fontHeight;
		}
		else
		{
			if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
				continue;

			switch (anchor)
			{
			case TextAnchor::LeftTop:
				draw_bitmap_outline(
					curPosX + font->slot->bitmap_left,
					curPosY - font->slot->bitmap_top
				);
				break;

			case TextAnchor::Center:
				draw_bitmap_outline(
					curPosX + font->slot->bitmap_left,
					curPosY - font->slot->bitmap_top
				);
				break;
			}

			curPosX += font->slot->advance.x >> 6;
		}
	}

	// 描画位置リセット
	CursorSet();

	// 2.本体を描画
	for (char32_t c : u32text_)
	{
		if (c == U'\n')
		{
			curPosX = static_cast<int>(position.x);
			curPosY += fontHeight;
		}
		else
		{
			if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
				continue;

			int r = (int)textLayout.textColor.x;
			int g = (int)textLayout.textColor.y;
			int b = (int)textLayout.textColor.z;

			switch (anchor)
			{
			case TextAnchor::LeftTop:
				draw_bitmap_core(
					curPosX + font->slot->bitmap_left,
					curPosY - font->slot->bitmap_top,
					r,g,b
				);
				break;

			case TextAnchor::Center:
				draw_bitmap_core(
					curPosX + font->slot->bitmap_left,
					curPosY - font->slot->bitmap_top,
					r, g, b
				);
				break;
			}

			curPosX += font->slot->advance.x >> 6;
		}
	}

	sprite.TextTextureUpdate(image);

	sprite.Draw();
}

// 左上、真ん中原点にカーソル初期位置やらをセット

void Text::CursorSet() {

	curPosX = 0;
	curPosY = 0;

	last_height = 0;

	fontHeight = font->face->size->metrics.height >> 6;

	// 左上原点
	if (anchor_ == TextAnchor::LeftTop) {

		sprite.SetPosition(position_);

		//
		int maxBitmapTop = 0;

		for (char32_t c : u32text_)
		{
			if (c == U'\n')
				continue;

			if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
				continue;

			maxBitmapTop = (std::max)(
				maxBitmapTop,
				static_cast<int>(font->slot->bitmap_top)
				);
		}

		curPosY = maxBitmapTop;


	}

	// 中央なら
	else if (anchor_ == TextAnchor::Center) {

		// 描画するテキスチャの中央に文字列を配置
		curPosX = (width - textWidth) / 2;

		int ascender = font->face->size->metrics.ascender >> 6;
		curPosY = (height - fontHeight) / 2 + ascender;

		// スプライトをposition中央にずらす

		Vector2 spriteCenter = {
			position_.x - (float)sprite.GetImageWidth() / 2.0f,
			position_.y - (float)sprite.GetImageHeight() / 2.0f,
		};

		sprite.SetPosition(spriteCenter);

	}
}

unsigned char* Text::getImage() {
	// 画像取得ロジック
	return image;
}

int Text::GetTextWidth(const std::string& text)
{
	std::u32string u32text = ToU32String(text);

	int width = 0;

	for (char32_t c : u32text)
	{
		if (FT_Load_Char(font->face, c, FT_LOAD_DEFAULT))
			continue;

		width += font->slot->advance.x >> 6;
	}

	return width;
}
int Text::GetTextHeight(const std::string& text)
{
	std::u32string u32text = ToU32String(text);

	int height = 0;

	for (char32_t c : u32text)
	{
		if (FT_Load_Char(font->face, c, FT_LOAD_DEFAULT))
			continue;

		height = (std::max)(height, static_cast<int>(font->slot->bitmap.rows));
	}

	return height;
}

//生成された位置も自分の画像データをimageにコピーする(RGBAにも変換するみたい)
void Text::draw_bitmap_outline(int x, int y)
{

	for (int ox = -textLayout.outlineSize; ox <= textLayout.outlineSize; ++ox)
	{
		for (int oy = -textLayout.outlineSize; oy <= textLayout.outlineSize; ++oy)
		{
			if (ox == 0 && oy == 0)
				continue;

			float distance = std::sqrt(
				static_cast<float>(ox * ox + oy * oy)
			);

			if (distance > textLayout.outlineSize)
				continue;

			float strength = 1.0f - distance / textLayout.outlineSize;

			int r = (int)textLayout.outlineColor.x;
			int g = (int)textLayout.outlineColor.y;
			int b = (int)textLayout.outlineColor.z;

			// もしぼかし無効なら
			if (textLayout.isOutlineBlur == false) strength = 1.0f;

			draw_bitmap_core(
				x + ox,
				y + oy,
				r, g, b,
				static_cast<int>( 255.0f * strength)
			);
		}
	}

}

void Text::draw_bitmap_core(
	int x, int y,
	int r, int g, int b,
	unsigned char alphaScale)
{
	const int x_max = x + font->slot->bitmap.width;
	const int y_max = y + font->slot->bitmap.rows;

	for (int i = x, p = 0; i < x_max; i++, p++)
	{
		for (int j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 || i >= width || j >= height)
				continue;

			unsigned char alpha =
				font->slot->bitmap.buffer[
					q * font->slot->bitmap.pitch + p
				];

			if (alpha == 0)
				continue;

			// 縁取り側の透明度を掛ける
			alpha = static_cast<unsigned char>(
				static_cast<float>(alpha) *
				static_cast<float>(alphaScale) / 255.0f
				);

			int index = (j * width + i) * 4;

			image[index + 0] = r;
			image[index + 1] = g;
			image[index + 2] = b;

			image[index + 3] =
				(std::max)(image[index + 3], alpha);
		}
	}
}

std::u32string ToU32String(const std::string& str)
{
	if (str.empty())
		return {};

	int size = MultiByteToWideChar(
		CP_UTF8,
		0,
		str.data(),
		static_cast<int>(str.size()),
		nullptr,
		0
	);

	if (size <= 0)
		return {};

	std::wstring wstr(size, L'\0');

	MultiByteToWideChar(
		CP_UTF8,
		0,
		str.data(),
		static_cast<int>(str.size()),
		wstr.data(),
		size
	);

	std::u32string result;

	for (size_t i = 0; i < wstr.size(); ++i)
	{
		wchar_t c = wstr[i];

		// UTF-16のサロゲートペア
		if (c >= 0xD800 && c <= 0xDBFF && i + 1 < wstr.size())
		{
			wchar_t c2 = wstr[i + 1];

			if (c2 >= 0xDC00 && c2 <= 0xDFFF)
			{
				char32_t codePoint =
					0x10000
					+ ((c - 0xD800) << 10)
					+ (c2 - 0xDC00);

				result.push_back(codePoint);
				++i;
				continue;
			}
		}

		result.push_back(static_cast<char32_t>(c));
	}

	return result;
}