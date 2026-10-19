#pragma once

#include <string>

#include "EasingVec.h"

#include "Vector.h"
#include "WorldTransform.h"

// ブレンドモード
enum class BlendMode {

	//!くブレンドなし
	kBlendModeNone,
	//!く通常αブレンド。デフォルト。 Src * SrcA+Dest *(1-SrcA)
	kBlendModeNormal,
	// !< 加算。Src * SrcA+Dest* 1
	kBlendModeAdd,
	// !< 減算。Dest * 1- Src * SrcA
	kBlendModeSubtract,
	// !< 乗算。Src*0+Dest* Src
	kBlendModeMultily,
	// !< スクリーン。Src *(1-Dest)+Dest * 1
	kBlendModeScreen,

	// カウント用
	Count,

};

// 粒子
struct ParticleEmit {

	std::string tag;		// 登録されてるモデルのタグ
	std::string textureTag; // 登録されてるテキスチャーのタグ

	WorldTransform transform; // 座標

	Vector3 direction;						 // 方向
	Vector3 velocity;						 // 移動量
	Vector4 color;						     // 色( のアドレス)
	float speed;							 // スピード

	float gravity = 0.0f;	     			 // 重力

	bool isBillboard = true;				 // 板ポリ用　常にカメラ目線
	bool isDilute = true;				     // 色を薄くしていくか
	bool isActive = false;

	float life = 20;						 // 生存時間
	float timer = 0;

	EasingVec ease;						// 減速用 
	EasingVec colorEase;				// 薄くする用 
	
	BlendMode blendType = BlendMode::kBlendModeNone;// ブレンドタイプ

	void init() {
		ease.Reset();
		ease.Start(EasingVec::EASE_OutSine, life, speed, 0);

		colorEase.Reset();
		colorEase.Start(EasingVec::EASE_None, life, 1.0f, 0.0f);
	}
	void update() {

		if (isActive) {

			ease.Step();
			speed = ease.getEasingNum();
			
			// 徐々に透明
			if (isDilute) {
				colorEase.Step();
				color.w = colorEase.getEasingNum();
			}

			velocity = direction * speed;

			// 重力分毎フレーム加算（下に落下）
			velocity.y -= gravity;

			transform.translation_.x += velocity.x;
			transform.translation_.y += velocity.y;
			transform.translation_.z += velocity.z;

			transform.UpdateMatrix();
		}

		timer++;

		// 消える
		if (timer >= life) {

			// 小さくして消す
			transform.scale_ = { 0,0,0 };

			isActive = false;
		}
	}

};