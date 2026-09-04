#pragma once

#include"Vector.h"

#include "Particle.h"
#include <string>
#include <map>

// エフェクトの初期化や更新など


// ┌─────────────────────────────┐
// │         Effect System（なにこれ       
// │                             │
// │ EffectManager               │
// │   └─ Effect                 │
// │        └─ Particle          │
// │                             │
// │ Particle                    │
// │   └─ パーティクル用の       │
// │   モデルをResourceを        │
// │                共有・保存   │
// │                             │
// │   ★それをEffect内で        │
// │　　　　　　宣言して使う     │
// │                             │
// └─────────────────────────────┘

//  リソース管理もパーティクルクラス内であるからそれを使う的なあれ


//struct ParticleCompornentParam {
//	Vector3 position = { 0,0,0 };
//	int num = 1;
//	ParticleParam param_;
//};

// エフェクト内のパーティクルの物の詳細
struct ParticleCompornent {

	std::string tag;			// particleクラスに登録されてるほうのタグ

	Vector3 position = { 0,0,0 };
	int num = 1;
	ParticleParam param_;

};


class Effect
{
public:

	virtual ~Effect() = default;

	virtual void Initialize(Vector3 position) = 0;
	virtual void Update() = 0;

//	virtual void Draw() = 0;　既にパーティクルの方で描画処理はあるので・・


	// エフェクト終了か
	bool IsFinished() { return isFinished_; };



	// ImGuiの識別ID
	void SetImGuiId(int id) {
		imguiId_ = id;
	}

protected:

	// エフェクト発生する座標
	Vector3 pos; 

	// 修了確認処理
	bool isFinished_ = false;


	

	// デバッグ用表示するか
	bool isImGuiView_ = false;


	// 登録
	void RegionParticleCompornent(std::string tag, std::string path);

	// 発生
	void EmitParticleCompornent(std::string tag) {

		if (!particleEmitParams_.contains(tag)) return;

		Particle::GetInstance()->MakeNewParticle(
			particleEmitParams_[tag].tag,
			particleEmitParams_[tag].position,
			particleEmitParams_[tag].num,
			particleEmitParams_[tag].param_);
	};

	// ImGui表示用
	void ImGuiViewParticleCompornent();

	// 扱いやすいようにしたかった
	std::map<std::string, ParticleCompornent> particleEmitParams_;
private:

	// ImGuiの識別ID　これがないと新しくつくったEffectに色々同期されてしまうので っていうかprivateでいいんだ
	int imguiId_ = 0;

};	
