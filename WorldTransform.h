#pragma once

#include "Transform.h"

#include "Matrix.h"

// 3D用
struct WorldTransform : public Transform {

	void Initialize() {

		// WorldViewProjection用のリソース
		wvpResource_ = DirectXUtility::CreateBufferResource(DirectXCommon::GetInstance()->GetDevice().Get(), sizeof(TransformationMatrix));

		//書き込むためのアドレスを取得
		wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&mvpData_));

		//単位行列を書きこんでおく
		*mvpData_ = { MakeIdentity4x4(),MakeIdentity4x4() };

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
	};

	TransformationMatrix* mvpData_ = nullptr;

};
