#pragma once

#include "Vector.h"
#include "Matrix.h"

#include "DirectXUtility.h"
#include "DirectXCommon.h"

#include <wrl.h>

#include <d3d12.h>

// シェーダーに転送用
struct TransformationMatrix {

	Matrix4x4 WVP;
	Matrix4x4 World;

};

struct Transform {

	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	Vector3 translation_ = { 0.0f,0.0f,0.0f };

	Matrix4x4 matWorld_;


	// wvpのマップ
	virtual void Initialize() = 0;


	// 親子の親の方のアドレス
	Transform* parent_ = nullptr;

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetWvpResource() const { return wvpResource_; }

protected:

	// 自分専用
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

};