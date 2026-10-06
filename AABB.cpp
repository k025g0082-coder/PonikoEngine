#include "AABB.h"
#include "ImGuiManager.h"

void AABB::ImGuiView() {

#ifdef USE_IMGUI
	ImGui::Separator();
	ImGui::DragFloat3("min: ", &min.x, 0.1f);
	ImGui::DragFloat3("max: ", &max.x, 0.1f);
	ImGui::Separator();
#endif

}

// 箱同士AABBの衝突
bool IsCollision(const AABB& aabb1, const AABB& aabb2) {
	if ((aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x#
		(aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y#
		(aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z)) { // z#
		// 衝突
		return true;
	}
	return false;
};