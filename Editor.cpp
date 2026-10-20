
#include "Editor.h"


#include "WinApp.h"

#include "Game.h"
#include "Scene.h"


#include "ImGuiManager.h"

#include "LightingManager.h"
#include "SceneComponentLoader.h"

#include "FileDropManager.h"

#include "Input.h"

#include "Particle.h"
#include "EffectManager.h"

#include "DebugLine.h"

#include "Vector.h"
#include "Matrix.h"

#include <algorithm>


Editor* Editor::GetInstance() {
	static Editor instance;
	return &instance;
}

/* エディター初期化 */
void Editor::Initialize() {

	camera_.Initialize();

	currentSceneJsonPath = "resources/Scene/test.json";
	HierarchyLoad(currentSceneJsonPath);

	effectSequencer_.mFrameMin = -100;
	effectSequencer_.mFrameMax = 1000;
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 0, 10, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 1, 20, 30 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 3, 12, 60 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 2, 61, 90 });
	effectSequencer_.items_.push_back(EffectSequencer::SequenceItem{ 4, 90, 99 });

}

/* エディター更新 */
void Editor::Update()
{

	// エディターカメラが有効なときだけ移動・反映する
	/*if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {

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
	}*/

	// 球面座標系？
	//camera_.SphericalCameraUpdate(
	//	selectedModel_->GetWorldTransform().translation_,
	//	camDistance,		  // 距離
	//	0.3f,                 // 上下角
	//	1.57f                 // 水平角（約90度）
	//);

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
				
				// 1) カメラを、選択したモデルのワールド座標、注視点とする
				// 2) カメラのviewMatrixからワールド座標を摘出する
				// 3) 選択モデルとカメラの距離を計算（ImViewGizmoの引数に必要）
				// 4) 距離から球面座標系に必要なthita横回転とphi縦を摘出
				// 5) 球面座標としてカメラのviewにセット、thitaとｐひもImViewGizmoにあげる
				
				Vector3 targetPos = model->GetWorldTransform().translation_;

				// 1) View行列からカメラのワールド行列を復元
				Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

				// 2) ワールド座標を取り出す
				Vector3 camPos = {
					cameraWorld.m[3][0],
					cameraWorld.m[3][1],
					cameraWorld.m[3][2]
				};

				// 3)距離
				selectedModelLength_ = Distance(camPos, targetPos);
				
				// 4) thetaとphi
				Vector3 offset = camPos - targetPos;
				selectedSphereTheta_ = std::asin(offset.y / selectedModelLength_);
				selectedSpherePhi_ = std::atan2(offset.z, offset.x);

				// 5) view更新！！
				camera_.SphericalCameraUpdate(
					targetPos,
					selectedModelLength_,
					selectedSphereTheta_,
					selectedSpherePhi_
				);

				break;
			}
		}

	}

	/* エフェクトタイムライン */
	EffectEditorTimeLine();

	// ImGuizmoのフレーム開始
	ImGuizmo::BeginFrame();

	

	ImGuizmo::Enable(true);
	// 3. カメラの投影モードを設定（透視投影の場合は false）
	ImGuizmo::SetOrthographic(false);

	float w = static_cast<float>(WinApp::GetInstance()->GetClientWidth());
	float h = static_cast<float>(WinApp::GetInstance()->GetClientHeight());

	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	ImGuizmo::SetRect(0.0f, 0.0f, w, h);

	/* カメラ視点の回転 */

	/* ホイール操作 */
	if (Input::GetInstance()->GetMouseWheel() != 0) {
		if (selectedModel_) {
			
			selectedModelLength_ += static_cast<float>(Input::GetInstance()->GetMouseWheel()) * 0.01f;

			// 近いの遠いの防止
			selectedModelLength_ = std::clamp(selectedModelLength_, 1.0f, 50.0f);

			// 1) カメラを、選択したモデルのワールド座標、注視点とする
			// 2) カメラのviewMatrixからワールド座標を摘出する
			// 3) 選択モデルとカメラの距離を計算（ImViewGizmoの引数に必要）
			// 4) 距離から球面座標系に必要なthita横回転とphi縦を摘出
			// 5) 球面座標としてカメラのviewにセット、thitaとｐひもImViewGizmoにあげる

			Vector3 targetPos = selectedModel_->GetWorldTransform().translation_;

			// 1) View行列からカメラのワールド行列を復元
			Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

			// 2) ワールド座標を取り出す
			Vector3 camPos = {
				cameraWorld.m[3][0],
				cameraWorld.m[3][1],
				cameraWorld.m[3][2]
			};

			// 4) thetaとphi
			Vector3 offset = camPos - targetPos;
			selectedSphereTheta_ = std::asin(offset.y / selectedModelLength_);
			selectedSpherePhi_ = std::atan2(offset.z, offset.x);

			// 5) view更新！！
			camera_.SphericalCameraUpdate(
				targetPos,
				selectedModelLength_,
				selectedSphereTheta_,
				selectedSpherePhi_
			);


		}
	}

	// 現在のカメラView行列を取得
	Matrix4x4 viewMatrix = camera_.GetViewMatrix();

	ImGuizmo::ViewManipulate(
		&viewMatrix.m[0][0],
		selectedModelLength_,
		ImVec2(ImGui::GetIO().DisplaySize.x - 256.0f, 0.0f),
		ImVec2(256.0f, 256.0f),
		0x10101010
	);

	// 変更されたView行列をそのまま反映
	camera_.SetViewMatrix(viewMatrix);

	


	if (isEditorMode_) {

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

			// 押された瞬間,メインカメラからviewをコピー
			if (isEditorMode_) {
				camera_.SetCamera(*Scene::GetSceneCamera());
			}

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

			ImGui::SameLine();
			ImGui::Text("hoi-ru %d", Input::GetInstance()->GetMouseWheel());
			ImGui::Text("targetleng %f", selectedModelLength_);
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

	// もしデバッグモードであれば、デバッグカメラのviewをメインカメラにコピー
	if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {
		Scene::GetSceneCamera()->SetCamera(camera_);
	}
}

// エディター描画
void Editor::Draw() {

	// シーンカメラが設定されてれば
	if (Scene::GetSceneCamera() != nullptr) {


		// モデル描画
		for (int i = 0; i < hierarchyModels_.size(); i++)
		{
			if (isEditorMode_) {
				hierarchyModels_.at(i).object->Draw(camera_);
			}
			// エディターモードじゃなければ、シーンのカメラで描画する
			else {
				hierarchyModels_.at(i).object->Draw(*Scene::GetSceneCamera());
			}
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
			h.path_ = relativePath;
			h.name_ = FileDropManager::GetFileNameWithoutExtension(path);// 表示名 拡張子を抜いたのを設定する
			h.object = model;				 // Model*

			hierarchyModels_.push_back(h);

		}

	}

#ifdef USE_IMGUI

	if (ImGui::Begin("Hierarchy", &show)) {

		/* 今開いてるシーン名 */
		ImGui::Text("SceneName: %s",currentSceneName_);

		// ロードボタン
		if (ImGui::Button("Load")) {
			HierarchySave(currentSceneJsonPath);
		}

		// セーブボタン
		if (ImGui::Button("Save")) {
			HierarchySave(currentSceneJsonPath);
		}

		// === モデルたち
		ImGui::SeparatorText("Models");

		for (int i = 0; i < hierarchyModels_.size(); i++)
		{
			if (ImGui::TreeNode(hierarchyModels_.at(i).name_.c_str())) {

				// ここに何か書く

				ImGui::TreePop();
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 40.0f);

			// 削除ボタンを右寄りに配置（↑のおかげ
			if (ImGui::Button("delete")) {
				hierarchyModels_.erase(hierarchyModels_.begin() + i);
				break;
			}
		}

		// === ライトたち
		ImGui::SeparatorText("Lights");

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

void Editor::HierarchyLoad(const std::string jsonPath) {
	// 読込み
	if (SceneComponentLoader::GetInstance()->Load(jsonPath)) {

		SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

		// ライト配置のロード
		SceneComponentLoader::GetInstance()->LightLoad();

		loder->BeginGroup("Hierarchy");

		// ヒエラルキのモデルリスト 登録
		loder->Property("Models",hierarchyModels_);

		loder->EndGroup();

		loder->LoadGroup("Hierarchy");
	}
}

void Editor::HierarchySave(const std::string jsonPath) {

	// ※注　ポインタ
	SceneComponentLoader* loder = SceneComponentLoader::GetInstance();

	json& data = loder->GetJson();

	// 必ず
	SceneComponentLoader::GetInstance()->ClearJson();

	// ライト配置のセーブ
	SceneComponentLoader::GetInstance()->LightSave();

	loder->SaveGroup("Hierarchy");

	SceneComponentLoader::GetInstance()->Save(jsonPath);
}

void Editor::EffectEditorTimeLine()
{

#ifdef USE_IMGUI

	// グリッド？ヤバ
	//ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);

	ImGui::Begin("EffectTimeline");

	// let's create the sequencer
	static int selectedEntry = -1;
	static int firstFrame = 0;
	static bool expanded = true;
	static int currentFrame = 100;

	ImGui::PushItemWidth(130);
	ImGui::InputInt("Frame Min", &effectSequencer_.mFrameMin);
	ImGui::SameLine();
	ImGui::InputInt("Frame ", &currentFrame);
	ImGui::SameLine();
	ImGui::InputInt("Frame Max", &effectSequencer_.mFrameMax);
	ImGui::PopItemWidth();
	Sequencer(&effectSequencer_, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
	
	// add a UI to edit that particular item
	if (selectedEntry != -1)
	{
		const EffectSequencer::SequenceItem& item = effectSequencer_.items_[selectedEntry];
		ImGui::Text("I am a %s, please edit me", effectSequencer_.GetSequencerItemTypeNames(item.mType));
		// switch (type) ....
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