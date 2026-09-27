#include "EffectManager.h"
#include "Particle.h"

std::vector<Effect*> EffectManager::effects_{};
int EffectManager::nextEffectId_ = 0;

void EffectManager::AddEffect(Effect* effect, Vector3 position,bool isDebugMode) {

    // 初期化
    effect->Initialize(position);

    // デバッグモードか設定
    effect->SetDebugMode(isDebugMode);

    effect->SetImGuiId(nextEffectId_++);

	// 追加
	effects_.push_back(effect);

}

void EffectManager::Update() {

    for (auto it = effects_.begin(); it != effects_.end();) {
        
        // 更新
        (*it)->Update();

        // エフェクト終了なら
        if ((*it)->IsFinished()) {
            delete* it;
            it = effects_.erase(it);
        }
        else {
            ++it;
        }
    }

}

// すべてのエフェクト終了
void EffectManager::ResetEffect() {
    for (Effect* e : effects_) {
        e->FinishEffect();
    }
}