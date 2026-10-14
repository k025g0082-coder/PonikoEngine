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