#pragma once
#include "Transform.h"

#include "ImGuiManager.h"

// 2Dよう
struct ScreenTransform : public BaseTransform{

	void Initialize() {

		//Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
		wvpResource_ = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(Matrix4x4));

		//書き込むためのアドレスを取得
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&mvpData_));

		//単位行列を書きこんでおく
		*mvpData_ = MakeIdentity4x4();

	}

	// アフィン行列更新
	void UpdateMatrix() {

		Matrix4x4 localMatrix =
			MakeAffineMatrix(scale_, rotation_, translation_);

		// 親がいたらそのぶん掛ける
		if (parent_) {
			matWorld_ = Multiply(localMatrix, parent_->matWorld_);
		}
		else {
			matWorld_ = localMatrix;
		}

	}

	Matrix4x4* mvpData_ = nullptr;

	// imgui用
	void ImGuiView(const char* label) {

#ifdef USE_IMGUI
		if (ImGui::TreeNode(label)) {
			ImGui::DragFloat3("translation", &translation_.x, 0.5f);
			ImGui::DragFloat3("rotation", &rotation_.x, 0.1f);
			ImGui::DragFloat3("scale", &scale_.x, 0.5f);
			ImGui::TreePop();
		}
#endif
	}
};