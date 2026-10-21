#pragma once

// 描画先の設定
#include "RenderTarget.h"

#include "Model.h"
#include "Camera.h"

#include <vector>
#include <memory>

/*
	SceneViewportクラス。
*/

/*
	RenderTargetクラスで受け取ったSRVハンドルを基に、登録されたモデルやカメラを描画させるよ
*/

class SceneViewport
{
public:

	void Initialize(uint32_t width, uint32_t height);
	void Update();
	void Draw();

	/* 使うモデルの追加 */
	void AddModel(Model* m) {
		models_.push_back(m);
	}

	// uniquePtrはgetでアドレスらしい　便利だね	
	RenderTarget* GetRenderTarget() { return renderTarget_.get(); }

private:

	// 破棄も自動でやってくれる素晴らしい機能
	std::unique_ptr<RenderTarget> renderTarget_;

	/* このビューポート内のカメラ */
	Camera camera_;

	std::vector<Model*> models_;

	uint32_t width_;
	uint32_t height_;
};

