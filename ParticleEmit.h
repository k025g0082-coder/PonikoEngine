#pragma once
#include "EasingVec.h"

// 粒子
struct ParticleEmit {

	std::string tag; // 登録されてるモデルのタグ

	WorldTransform transform; // 座標

	Vector3 direction;						 // 方向
	Vector3 velocity;						 // 移動量
	Vector4 color;						     // 色( のアドレス)
	float speed;							 // スピード

	float gravity = 0.0f;	     			 // 重力

	bool isBillboard = true;				 // 板ポリ用　常にカメラ目線
	bool isActive = false;

	float life = 20;						 // 生存時間
	float timer = 0;

	EasingVec ease;						// 減速用 
	
	void init() {
		ease.Reset();
		ease.Start(EasingVec::EASE_OutSine, life, speed, 0);
	}
	void update() {

		if (isActive) {

			ease.Step();
			speed = ease.getEasingNum();

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