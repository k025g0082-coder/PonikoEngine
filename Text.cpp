
#include "Text.h"

#include "WinApp.h"
#include "DirectXUtility.h"
#include "DirectXCommon.h"

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

// 文字列描画
void Text::drawString(const std::u32string& text, Vector2 position, TextAnchor anchor) {

	// キャンバス初期化？
	memset(image, 0, width * height * 4);

	int curPosX = static_cast<int>(position.x);
	int curPosY = static_cast<int>(position.y);

	int last_height = 0; //最後に文字を書いたときの文字の大きさ

	int fontHeight = font->face->size->metrics.height >> 6;

	int textWidth = 0;

	// 中央なら
	if (anchor == TextAnchor::Center) {

		// 全体の横幅取得
		for (char32_t c : text)
		{
			if (FT_Load_Char(font->face, c, FT_LOAD_RENDER))
				continue;

			textWidth += font->slot->advance.x >> 6;
		}

		// 開始位置を設定
		curPosX -= textWidth / 2;

		int ascender = font->face->size->metrics.ascender >> 6;
		curPosY = curPosY - fontHeight / 2 + ascender;

	}

	for (char32_t c : text)
	{
		if (c == U'\n')
		{
			curPosX = static_cast<int>(position.x);
			curPosY += fontHeight;
		}
		else
		{
			if (FT_Load_Char(font->	face, c, FT_LOAD_RENDER))
				continue;

			switch (anchor) {

				// 左上原点
			case TextAnchor::LeftTop:
				draw_bitmap(
					curPosX + font->slot->bitmap_left,
					curPosY + (fontHeight - font->slot->bitmap_top) - 50
				);
				break;

				// 中心
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

	// hyouji you sprite
	sprite.Draw();
}

unsigned char* Text::getImage() {
	// 画像取得ロジック
	return image;
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
				font->slot->bitmap.buffer[q * font->slot->bitmap.width + p];

			// RGBAのインデックス
			int index = (j * width + i) * 4;

			image[index + 0] = 255;   // R
			image[index + 1] = 255;   // G
			image[index + 2] = 255;   // B
			image[index + 3] = alpha; // A
		}
	}
}