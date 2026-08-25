#pragma once

#include "PipelineStateManager.h"

#include "Texture.h"

#include "Matrix.h"
#include "ScreenTransform.h"

class Sprite {
public:

	void Initialize(std::string texturePath);

	void TextTextureInitialize(unsigned char* image, int width, int height);

	void TextTextureUpdate(unsigned char* image);

	void Draw();
	void Draw(ScreenTransform& screenTransform);

	void SetPos(Vector3 pos) {
		
		screenTransform_.translation_ = pos;

		// 更新
		screenTransform_.UpdateMatrix();
	}


	void SetPosition(Vector2 pos) {
		screenTransform_.translation_.x = pos.x;
		screenTransform_.translation_.y = pos.y;

		// 更新
		screenTransform_.UpdateMatrix();
	}

	void SetRotation(float r) {
		screenTransform_.rotation_.z = r;
	}


	enum class anchor{
		leftTop, // 左上
		Center   // 中心
	};

	anchor spriteAnchor = anchor::leftTop;

	// アンカーセット
	void SetAnchor(anchor a) { 

		spriteAnchor = a; 

		// 再設定
		SetSize(imageWidth, imageHeight);
	}

	// サイズ変更
	void SetSize(int width,int height) {

		// ﾒﾝﾊﾞに設定
		imageWidth = width;
		imageHeight = height;

		float tWidth = static_cast<float>(width);
		float tHeight = static_cast<float>(height);

		switch (spriteAnchor) {

		case anchor::leftTop:

			// 1枚目の三角形
			vertexDataSprite[0].position = { 0.0f, tHeight, 0.0f, 1.0f }; // 左下
			vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

			vertexDataSprite[1].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
			vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

			vertexDataSprite[2].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
			vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

			// 2枚目の三角形
			vertexDataSprite[3].position = { 0.0f,    0.0f,    0.0f, 1.0f }; // 左上
			vertexDataSprite[3].texcoord = { 0.0f, 0.0f };

			vertexDataSprite[4].position = { tWidth,  0.0f,    0.0f, 1.0f }; // 右上
			vertexDataSprite[4].texcoord = { 1.0f, 0.0f };

			vertexDataSprite[5].position = { tWidth,  tHeight, 0.0f, 1.0f }; // 右下
			vertexDataSprite[5].texcoord = { 1.0f, 1.0f };
			break;

		case anchor::Center:
			float left = -tWidth * 0.5f;
			float right = tWidth * 0.5f;
			float top = -tHeight * 0.5f;
			float bottom = tHeight * 0.5f;

			// 1枚目
			vertexDataSprite[0].position = { left,  bottom, 0.0f, 1.0f };
			vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

			vertexDataSprite[1].position = { left,  top,    0.0f, 1.0f };
			vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

			vertexDataSprite[2].position = { right, bottom, 0.0f, 1.0f };
			vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

			// 2枚目
			vertexDataSprite[3].position = { left,  top,    0.0f, 1.0f };
			vertexDataSprite[3].texcoord = { 0.0f, 0.0f };

			vertexDataSprite[4].position = { right, top,    0.0f, 1.0f };
			vertexDataSprite[4].texcoord = { 1.0f, 0.0f };

			vertexDataSprite[5].position = { right, bottom, 0.0f, 1.0f };
			vertexDataSprite[5].texcoord = { 1.0f, 1.0f };
			break;
		}
	}

	void SetColorRGBA(const Vector4& col) {
		*materialData = col;
	}

	void SetScreenTransform(ScreenTransform screenTransform) { screenTransform_ = screenTransform; }
	ScreenTransform GetScreenTransform() { return screenTransform_; }
	
	// 行列更新
	void UpdateScreenTransform() {
		screenTransform_.UpdateMatrix();
	}

private:

	int imageWidth = 0;
	int imageHeight = 0;

	// ScreenTransform,扱いやすくメンバ変数のほう
	ScreenTransform screenTransform_;

	// テクスチャ
	Texture texture;

	ID3D12Device* device_;

	// 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// 0201 マテリアル用のリソース
	ID3D12Resource* materialResource = nullptr;

	ID3D12Resource* vertexResource = nullptr;
	SpriteVertexData* vertexDataSprite = nullptr;

	Vector4* materialData = nullptr;
};

