
#include "Editor.h"


#include "WinApp.h"

#include "Game.h"
#include "Scene.h"


#include "ImGuiManager.h"
#include "LightingManager.h"

#include "FileDropManager.h"

#include "Input.h"

#include "Particle.h"
#include "EffectManager.h"

#include "DebugLine.h"

#include "Vector.h"
#include "Matrix.h"


Editor* Editor::GetInstance() {
	static Editor instance;
	return &instance;
}

/* エディター初期化 */
void Editor::Initialize() {
	camera_.Initialize();
}

/* エディター更新 */
void Editor::Update()
{

	// エディターカメラが有効なときだけ移動・反映する
	if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {

		float speed = 0.2f;
		Vector3 move = { 0.0f, 0.0f, 0.0f };

		if (Input::GetInstance()->PushKey(DIK_W)) move.y += speed;
		if (Input::GetInstance()->PushKey(DIK_S)) move.y -= speed;
		if (Input::GetInstance()->PushKey(DIK_A)) move.x -= speed;
		if (Input::GetInstance()->PushKey(DIK_D)) move.x += speed;
		if (Input::GetInstance()->PushKey(DIK_Q)) move.z -= speed;
		if (Input::GetInstance()->PushKey(DIK_E)) move.z += speed;

		auto& transform = camera_.GetWorldTransform();
		transform.translation_.x += move.x;
		transform.translation_.y += move.y;
		transform.translation_.z += move.z;

		camera_.Update();

		Scene::GetSceneCamera()->SetCamera(camera_);
	}
	

	// モデル更新
	for (int i = 0; i < hierarchyModels_.size(); i++)
	{
		hierarchyModels_.at(i).object->Update();
	}
	

#ifdef USE_IMGUI

	static bool showHierarchy = true;
	static bool showLighting = true;


	ImVec2 scenePos = ImGui::GetCursorScreenPos();
	ImVec2 sceneSize = ImGui::GetContentRegionAvail();

	ImVec2 mouse = ImGui::GetMousePos();

	float width = static_cast<float>(
		WinApp::GetInstance()->GetClientWidth());

	float height = static_cast<float>(
		WinApp::GetInstance()->GetClientHeight());

	bool insideScene =
		mouse.x >= 0.0f &&
		mouse.x <= width &&
		mouse.y >= 0.0f &&
		mouse.y <= height;

	if (insideScene &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
		!ImGui::GetIO().WantCaptureMouse &&
		!ImGuizmo::IsOver() &&
		!ImGuizmo::IsUsing()) {

		// モデル選択処理
		float ndcX = (mouse.x / width) * 2.0f - 1.0f;
		float ndcY = 1.0f - (mouse.y / height) * 2.0f;

		// レイ生成
		Camera::Ray ray = camera_.ScreenPointToRay(ndcX, ndcY);

		float distance = 0.0f;


		selectedModel_ = nullptr;

		// すべてのモデル
		for (int i = 0; i < hierarchyModels_.size(); i++)
		{

			Model* model = hierarchyModels_[i].object;

			// 大きさとか作ってないので ひとまず１，１で
			AABB aabb;
			aabb.min = Vector3{ -0.5f, -0.5f, -0.5f } + model->GetWorldTransform().translation_;
			aabb.max = Vector3{ 0.5f,  0.5f,  0.5f } + model->GetWorldTransform().translation_;

			// モデルのAABBと判定
			if (camera_.IntersectRayAABB(ray, aabb, distance)) {
				selectedModel_ = model;
				break;
			}
		}

	}


	// ImGuizmoのフレーム開始
	ImGuizmo::BeginFrame();

	

	ImGuizmo::Enable(true);
	// 3. カメラの投影モードを設定（透視投影の場合は false）
	ImGuizmo::SetOrthographic(false);

	float w = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	float h = static_cast<float>(WinApp::GetInstance()->GetClientHeight());

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::SetRect(0.0f, 0.0f, w, h);




		// 選択中のモデルがあるときだけ操作
	if (selectedModel_) {

		// 転置なしでそのまま渡す
		Matrix4x4 viewMat = camera_.GetViewMatrix();
		Matrix4x4 projMat = camera_.GetProjectionMatrix();
		Matrix4x4 worldMat = selectedModel_->GetWorldTransform().matWorld_;

		bool isManipulated = ImGuizmo::Manipulate(
			&viewMat.m[0][0],
			&projMat.m[0][0],
			operation_,
			ImGuizmo::WORLD,
			&worldMat.m[0][0]
		);

		if (isManipulated || ImGuizmo::IsUsing()) {

			switch (operation_) {
			case ImGuizmo::TRANSLATE:
				selectedModel_->GetWorldTransform().translation_.x = worldMat.m[3][0];
				selectedModel_->GetWorldTransform().translation_.y = worldMat.m[3][1];
				selectedModel_->GetWorldTransform().translation_.z = worldMat.m[3][2];
				break;
			case ImGuizmo::ROTATE:
				selectedModel_->GetWorldTransform().rotation_ = ExtractRotation(worldMat, selectedModel_->GetWorldTransform().scale_);
				break;
			case ImGuizmo::SCALE:
				selectedModel_->GetWorldTransform().scale_ = ExtractScale(worldMat);
				break;

			}


			selectedModel_->GetWorldTransform().UpdateMatrix();
		}
	}
	/* メニュー表示 */
	if (ImGui::BeginMainMenuBar()) {
	
		if (ImGui::BeginMenu("Window")) {

			ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
			ImGui::MenuItem("LightingManager", nullptr, &showLighting);
			ImGui::EndMenu();
		}

		// Window の横に配置
		ImGui::SameLine(60.0f);

		if (ImGui::Checkbox("Editor ", &isEditorMode_)) {

		//	 ON/OFF切り替え
			Scene::GetSceneCamera()->SetEditorMode(isEditorMode_);

		}
		ImGui::SameLine(200.0f);

		if (isEditorMode_) {

			if (ImGui::Button("Translate")) {
				operation_ = ImGuizmo::TRANSLATE;
			}

			ImGui::SameLine();

			if (ImGui::Button("Rotate")) {
				operation_ = ImGuizmo::ROTATE;
			}

			ImGui::SameLine();

			if (ImGui::Button("Scale")) {
				operation_ = ImGuizmo::SCALE;
			}
		}

		ImGui::EndMainMenuBar();
	}

	// ヒエラルキーパネル
	if (showHierarchy) {
		HierarchyTab(showHierarchy);
	}

	// ライティングパネル
	if (showLighting) {
		LightingManagerTab(showLighting);
	}

	//ImGui::End();
#endif
}

// エディター描画
void Editor::Draw() {

// シーンカメラが設定されてれば
	if (Scene::GetSceneCamera() != nullptr) {
			
		// モデル描画
		for (int i = 0; i < hierarchyModels_.size(); i++)
		{
			hierarchyModels_.at(i).object->Draw(camera_);
		}
	}

	if (isEditorMode_) {

		if (selectedModel_) {

			AABB aabb;
			aabb.min = Vector3{ -0.5f, -0.5f, -0.5f } + selectedModel_->GetWorldTransform().translation_;
			aabb.max = Vector3{ 0.5f,  0.5f,  0.5f } + selectedModel_->GetWorldTransform().translation_;

			// 四角描画
			DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });

		}
	}


}

//
void Editor::HierarchyTab(bool& show) {

	// 新しくドロップされたパスを取得
	auto paths =
		FileDropManager::TakeFilePaths(FileType::kModel);

	for (const auto& path : paths) {

		// 絶対パス → resources/ 以下の相対パス
		std::string relativePath =
			FileDropManager::GetRelativePath(path);

		// 相対パスが取得できなければスキップ
		if (relativePath.empty()) {
			continue;
		}

		// モデル生成
		Model* model = Model::CreateOBJ(relativePath);

		if (model) {

			// ヒエラルキ追加
			Hierarchy<Model> h;

			h.type_ = HierarchyType::kModel; // タイプ
			h.name_ = FileDropManager::GetFileNameWithoutExtension(path);// 表示名 拡張子を抜いたのを設定する
			h.object = model;				 // Model*

			hierarchyModels_.push_back(h);

		}

	}

#ifdef USE_IMGUI
	if (ImGui::Begin("Hierarchy", &show)) {

		ImGui::Text("Models:");

		// モデル名
		for (int i = 0; i < hierarchyModels_.size(); i++)
		{
			ImGui::BulletText("%s", hierarchyModels_.at(i).name_.c_str());
		}

	}

	ImGui::End();
#endif
}
void Editor::LightingManagerTab(bool& show) {

#ifdef USE_IMGUI

	ImGui::Begin("Lighting-Manager", &show);

	if (ImGui::Button("Add_DirectionalLight")) {
		DirectionalLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}
	if (ImGui::Button("Add_SpotLight")) {
		SpotLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}
	if (ImGui::Button("Add_PointLight")) {
		PointLightParam param;
		LightingManager::GetInstance()->AddLight(param);
	}


	// === DirectionalLight
	if (ImGui::TreeNode("DirectionalLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::DragFloat3(
				"Direction",
				&param.lights[i].direction.x,
				0.1f
			);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat(
				"itensity",
				&param.lights[i].intensity,
				0.1f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().dLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// === SpotlLight
	if (ImGui::TreeNode("SpotLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat3(
				"Position",
				&param.lights[i].position.x,
				0.1f
			);

			ImGui::DragFloat(
				"Intensity",
				&param.lights[i].intensity,
				0.1f
			);

			ImGui::DragFloat3(
				"Direction",
				&param.lights[i].direction.x,
				0.1f
			);

			ImGui::DragFloat(
				"Inner Angle",
				&param.lights[i].innerAngle,
				0.1f
			);

			ImGui::DragFloat(
				"Outer Angle",
				&param.lights[i].outerAngle,
				0.1f
			);

			ImGui::DragFloat(
				"Distance",
				&param.lights[i].distance,
				0.1f
			);

			ImGui::DragFloat(
				"Falloff",
				&param.lights[i].falloff,
				0.1f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().sLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	// === PointlLight
	if (ImGui::TreeNode("PointLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			ImGui::ColorPicker4(
				"Color",
				&param.lights[i].color.x
			);

			ImGui::DragFloat3(
				"Position",
				&param.lights[i].position.x,
				0.1f
			);

			ImGui::DragFloat(
				"Intensity",
				&param.lights[i].intensity,
				0.1f,
				0.0f
			);

			ImGui::DragFloat(
				"Radius",
				&param.lights[i].radius,
				0.1f,
				0.0f
			);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (ImGui::Button("Delete"))
			{
				LightingManager::GetInstance()->GetLightSets().pLight->Remove(i);
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}


	ImGui::End();
#endif
}


Vector3 Editor::ExtractScale(const Matrix4x4& m)
{
	return {
		std::sqrt(
			m.m[0][0] * m.m[0][0] +
			m.m[0][1] * m.m[0][1] +
			m.m[0][2] * m.m[0][2]
		),
		std::sqrt(
			m.m[1][0] * m.m[1][0] +
			m.m[1][1] * m.m[1][1] +
			m.m[1][2] * m.m[1][2]
		),
		std::sqrt(
			m.m[2][0] * m.m[2][0] +
			m.m[2][1] * m.m[2][1] +
			m.m[2][2] * m.m[2][2]
		)
	};
}
Vector3 Editor::ExtractRotation(const Matrix4x4& m, const Vector3& scale)
{
	// スケールを除去
	float r00 = m.m[0][0] / scale.x;
	float r01 = m.m[0][1] / scale.x;
	float r02 = m.m[0][2] / scale.x;

	float r10 = m.m[1][0] / scale.y;
	float r11 = m.m[1][1] / scale.y;
	float r12 = m.m[1][2] / scale.y;

	float r20 = m.m[2][0] / scale.z;
	float r21 = m.m[2][1] / scale.z;
	float r22 = m.m[2][2] / scale.z;

	// MakeAffineMatrix の Rx * Ry * Rz に対応
	float ry = std::asin((std::clamp)(-r02, -1.0f, 1.0f));

	float cy = std::cos(ry);

	float rx;
	float rz;

	if (std::abs(cy) > 1.0e-6f) {
		rx = std::atan2(r12, r22);
		rz = std::atan2(r01, r00);
	}
	else {
		// ジンバルロック付近
		rx = std::atan2(-r21, r11);
		rz = 0.0f;
	}

	return { rx, ry, rz };
}