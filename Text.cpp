
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

	// 画面サイズ取得[
	width = 800; // 幅は固定
	height = 200; // 高さは固定

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

	//int curPosX = static_cast<int>(position.x);
	//int curPosY = static_cast<int>(position.y);

	int curPosX = 0;
	int curPosY = -28;

	int last_height = 0;

	int fontHeight = font->face->size->metrics.height >> 6;

	int textWidth = 0;

	// 中央なら
	if (anchor == TextAnchor::Center) {

		// 全体の横幅取得
		for (char32_t c : u32text)
		{
			if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
				continue;

			textWidth += font->slot->advance.x >> 6;
		}

		curPosX -= textWidth / 2;

		int ascender = font->face->size->metrics.ascender >> 6;
		curPosY = curPosY - fontHeight / 2 + ascender;
	}

	for (char32_t c : u32text)
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
				draw_bitmap(
					curPosX + font->slot->bitmap_left,
					curPosY + (fontHeight - font->slot->bitmap_top)
				);
				break;

			case TextAnchor::Center:
				draw_bitmap(
					curPosX + font->slot->bitmap_left,
					curPosY - font->slot->bitmap_top
				);
				break;
			}

			curPosX += font->slot->advance.x >> 6;
		}
	}

	sprite.TextTextureUpdate(image);

	sprite.SetPos(Vector3{ position.x, position.y, 0.0f });

	sprite.Draw();
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
void Text::draw_bitmap(int x, int y)
{
	int i, j, p, q;
	const int x_max = x + font->slot->bitmap.width;
	const int y_max = y + font->slot->bitmap.rows;

	for (i = x, p = 0; i < x_max; i++, p++)
	{
		for (j = y, q = 0; j < y_max; j++, q++)
		{
			if (i < 0 || j < 0 || i >= width || j >= height)
			{
				continue;
			}

			// FreeTypeが生成したグレースケール値
			unsigned char alpha =
				font->slot->bitmap.buffer[q * font->slot->bitmap.pitch + p];

			// RGBAのインデックス
			int index = (j * width + i) * 4;

			image[index + 0] = 255;   // R
			image[index + 1] = 255;   // G
			image[index + 2] = 255;   // B
			image[index + 3] = alpha; // A
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