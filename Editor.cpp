
#include "Editor.h"


#include "WinApp.h"

#include "Game.h"
#include "Scene.h"


#include "ImGuiManager.h"
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

#include "LightingManager.h"

#include "Input.h"

//#include "Particle.h"
//#include "EffectManager.h"

#include "FileDropManager.h"

#include "DebugLine.h"

#include "Vector.h"
#include "Matrix.h"

#include "Timer.h"

#include <algorithm>
#include <numbers>
#include <memory>
#include <format>
#include <optional>

HierarchyPanel Editor::hierarchyPanel_;
Grid Editor::grid;
std::string Editor::currentSceneJsonPath;

Editor* Editor::GetInstance() {
	static Editor instance;
	return &instance;
}

/* エディター初期化 */
void Editor::Initialize() {

	camera_.Initialize();

	
	// エディタの設定読込
	LoadIniFile();


	// 各パネルの初期化
	hierarchyPanel_.Initialize(currentSceneJsonPath,grid);

	assetsPanel_.Initialize();
	assetsPanel_.SetHierarchyPanel(hierarchyPanel_);



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

	// 新しくドロップされたシーンのJsonパスを取得
	auto paths =
		FileDropManager::TakeFilePaths(FileType::kJson);

	for (const auto& path : paths) {

		// 絶対パス → resources/ 以下の相対パス
		std::string relativePath =
			FileDropManager::GetRelativePath(path);

		// 相対パスが取得できなければスキップ
		if (relativePath.empty()) {
			continue;
		}

		/* 
			ここにエフェクト用のjsonだかを識別する何かをいつかかく
		*/
		LoadScene(relativePath);
		currentSceneJsonPath = relativePath;
	}


	// 起動時にGameでSceneカメラ設定終わってからEditorModeのカメラせっていしたかたったのでこんなんなりました
	if (!ini) {

		if (isEditorMode_ && Scene::GetSceneCamera()) {

			//* isEditor切り替えたときと一緒 *///
			WinApp::GetInstance()->SetDragEnabled(true);

			Scene::GetSceneCamera()->SetEditorMode(true);

			ini = true;
		}
		
	};


	/* 常に更新するパネルの内容 */
	hierarchyPanel_.Update();
	assetsPanel_.Update();


#ifdef USE_IMGUI

	static bool showProperty = false;
	static bool showHierarchy = false;
	static bool showLighting = false;
	static bool showEffectTimeline = false;
	static bool showAssets = false;

	/* メニュー表示 */
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Window")) {

			ImGui::MenuItem("Property", nullptr, &showProperty);
			ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
			ImGui::MenuItem("Assets", nullptr, &showAssets);
			ImGui::MenuItem("EffectTimeline", nullptr, &showEffectTimeline);
			ImGui::MenuItem("LightingManager", nullptr, &showLighting);

			ImGui::EndMenu();
		}

		// Window の横に配置
		ImGui::SameLine(60.0f);
		ImGui::Text("DebugMode:");
		ImGui::SameLine();

		/* エディターモード起動 */
		if (ToggleSwitch("Editor ", &isEditorMode_)) {

			// 押された瞬間,メインカメラからviewをコピー
			if (isEditorMode_) {
				
				////* ドロップ有効に
				WinApp::GetInstance()->SetDragEnabled(true);

				camera_.SetCamera(*Scene::GetSceneCamera());
			}
			else {
				////* ドロップ無効に
				WinApp::GetInstance()->SetDragEnabled(false);
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

			if (selectedModelWorldTransform_) {

				// もしヒエラルキ移動ボタン押されたら
				if (operation_ == ImGuizmo::TRANSLATE) {
					ImGui::SameLine();
					ImGui::Text("Translate:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::DragFloat3(" ", &selectedModelWorldTransform_->translation_.x, 0.1f);
				}
				// もしヒエラルキ回転ボタン押されたら
				if (operation_ == ImGuizmo::ROTATE) {
					ImGui::SameLine();
					ImGui::Text("Rotate:");
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200.0f);
					ImGui::DragFloat3(" ", &selectedModelWorldTransform_->rotation_.x, 0.1f);

					ImGui::SameLine();
					if (ImGui::Button("x")) {
						selectedModelWorldTransform_->rotation_.x -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
					ImGui::SameLine();
					if (ImGui::Button("y")) {
						selectedModelWorldTransform_->rotation_.y -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
					ImGui::SameLine();
					if (ImGui::Button("z")) {
						selectedModelWorldTransform_->rotation_.z -= std::numbers::pi_v<float> / 4.0f;
						selectedModelWorldTransform_->UpdateMatrix();
					}
				}
					// もしヒエラルキ拡大ボタン押されたら
					if (operation_ == ImGuizmo::SCALE) {
						ImGui::SameLine();
						ImGui::Text("Scale:");
						ImGui::SameLine();
						ImGui::SetNextItemWidth(200.0f);
						ImGui::DragFloat3(" ", &selectedModelWorldTransform_->scale_.x, 0.1f);
					}
					

				
			}
		}
		

		ImGui::EndMainMenuBar();
	}

	// プロパティー
	if (showProperty) {

		/* 色 */
		ImGui::PushStyleColor(
			ImGuiCol_WindowBg,
			ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
		);

		ImGui::Begin("Property",&showProperty);

		/* 今開いてるシーン名 */
		ImGui::Text("SceneName: %s", currentSceneName_.c_str());
		ImGui::SameLine();


		ImGui::InputText(" ", &currentSceneJsonPath);


		// ロードボタン
		if (ImGui::Button("Load")) {
			hierarchyPanel_.HierarchyLoad(currentSceneJsonPath);
		}

		ImGui::SameLine();

		// セーブボタン
		if (ImGui::Button("Save")) {
			hierarchyPanel_.HierarchySave(currentSceneJsonPath);
		}


		// ワールド座標系に
		Matrix4x4 viewMatrix = Inverse(camera_.GetViewMatrix());

		/* カメラから位置と回転を入手 */
		Vector3 cameraPosition;
		cameraPosition.x = viewMatrix.m[3][0];
		cameraPosition.y = viewMatrix.m[3][1];
		cameraPosition.z = viewMatrix.m[3][2];

		Vector3 cameraRotation = ExtractRotation(viewMatrix, { 1.0f,1.0f,1.0f });

		ImGui::Separator();

		ImGui::Text("Camera WorldTransrate x:%3.2f y:%3.2f z:%3.2f ", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		ImGui::Text("Camera WorldRotation x:%3.2f y:%3.2f z:%3.2f ", cameraRotation.x, cameraRotation.y, cameraRotation.z);

		// カメラ位置コピー（小癪な・・）
		if (ImGui::Button("Copy")) {

			// 少数第3位までフォーマット
			std::string tpos = "Vector3{" +
				std::format("{:.3f}", cameraPosition.x) + "f," +
				std::format("{:.3f}", cameraPosition.y) + "f," +
				std::format("{:.3f}", cameraPosition.z) + "f}";

			std::string trot = "Vector3{" +
				std::format("{:.3f}", cameraRotation.x) + "f," +
				std::format("{:.3f}", cameraRotation.y) + "f," +
				std::format("{:.3f}", cameraRotation.z) + "f}";

			std::string txt = tpos + "\n" + trot;

			ImGui::SetClipboardText(txt.c_str());
		}




		
		

		ImGui::Separator();

		// === グリッド設定
		ImGui::SeparatorText("Grid");

		// スナップ状態なら
		if (grid.isSnap_) {

			// 選択中は色変える
			if (currentModelHierarchyMode_ == ModelHierarchyMode::kCreateMode)
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 50, 255));

			bool b = ImGui::Button("MapCreateMode");

			if (currentModelHierarchyMode_ == ModelHierarchyMode::kCreateMode)
				ImGui::PopStyleColor();


			if (b) {
				// 切り替え
				currentModelHierarchyMode_ == ModelHierarchyMode::kSelectMode ?
					currentModelHierarchyMode_ = ModelHierarchyMode::kCreateMode :
					currentModelHierarchyMode_ = ModelHierarchyMode::kSelectMode;

			}
		}

		grid.ImGuiView();

		ImGui::ColorEdit4("tuuti backcol", &notification_.backCol_.x);
		ImGui::ColorEdit4("tuuti textcol", &notification_.textCol_.x);

		ImGui::End();

		// 色の
		ImGui::PopStyleColor();
	}

	/* 各パネルはEditormodeがONの場合表示させる */

	if (isEditorMode_) {

		// ヒエラルキーパネル
		if (showHierarchy) {
			hierarchyPanel_.ViewPanel(showHierarchy);
		}

		/* エフェクトタイムライン */
		if (showEffectTimeline) {
			EffectEditorTimeLine();
		}

		// ライティングパネル
		if (showLighting) {
			LightingManagerTab(showLighting);
		}

		// アセットパネル
		if (showAssets) {
			assetsPanel_.PanelView(showAssets);
		}
	}

#endif

	//*
	if (!isEditorMode_) return;

#ifdef USE_IMGUI

	if (selectedModelWorldTransform_) {

		// ペースト
		if (Input::GetInstance()->TriggerKey(DIK_V)) {

			selectedModelWorldTransform_ = hierarchyPanel_.PasteHierarchyModel();

			// コピー処理が成功したタイミングで呼ぶ
			notification_.PopText("Model Pasted!!",ImVec2(170.0f,30.0f));
		}

	}

	// 通知のビョーガ
	notification_.Draw();


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

	/* ImGuiの枠にかさならない */
	if (insideScene &&
		!ImGui::GetIO().WantCaptureMouse &&
		!ImGuizmo::IsUsing()) {

		/* 左クリックを押された際の処理 */
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {


			// スクリーンの大きさとスクリーン座標からから、レイ生成
			Camera::Ray ray = camera_.ScreenPointToRay(Vector2{ mouse.x,mouse.y }, width, height);

			float distance = 0.0f;

			hierarchyPanel_.SetSelectedHierarchyModelIndex(std::nullopt);
			selectedModelWorldTransform_ = nullptr;

			//* モード別の処理　*//
			switch (currentModelHierarchyMode_) {


			//* 選択モード
			case ModelHierarchyMode::kSelectMode:

				// すべてのモデル
				for (int i = 0; i < hierarchyPanel_.GetHierarchyModels().size(); i++)
				{
					// もし選択中のモデルアセットがあった場合
					if (!assetsPanel_.selectedHierarchyResource_.pathId_.empty()) {

						/* 選択中以外の、アセット以外のIDのヒエラルキーはとばす */
						if (hierarchyPanel_.GetHierarchyModels().at(i).id_ != assetsPanel_.selectedHierarchyResource_.pathId_)
							continue;
					}

					// 非表示なら選択もスキップ
					if (!hierarchyPanel_.GetHierarchyModels().at(i).isView_)
						continue;

					// ヒエラルキからワールドのポインタ取得！
					WorldTransform* world = static_cast<WorldTransform*>(hierarchyPanel_.GetHierarchyModels().at(i).transform_.get());

					// だいたいの中心
					float size = 1.0f;

					AABB aabb;
					aabb.min = Vector3{ -size, -size, -size } + world->translation_;
					aabb.max = Vector3{ size,  size,  size } + world->translation_;

					// モデルのAABBと判定
					if (camera_.IntersectRayAABB(ray, aabb, distance)) {

						hierarchyPanel_.SetSelectedHierarchyModelIndex(i);
						selectedModelWorldTransform_ = world;

						// ビューポートにモデルを追加
						//effectViewport_.AddModel(hierarchyModels_.at(i).object);

						// 選択中のモデルのview更新
						SelectModelUpdateView();

						break;
					}
				}
				break;

				//* 一気に配置モード
			case ModelHierarchyMode::kCreateMode:

				if (assetsPanel_.selectedHierarchyResource_.pathId_.empty()) break;

				// グリッドのplaneと判定
				std::optional<Vector3> hit = camera_.IntersectRayPlaneArea(ray, grid.plane_);

				// 当たったら
				if (hit.has_value()) {

					// マスの大きさで割ったのを設定する
					Vector3 subPosition = Grid::snapGrid(hit.value());

					bool break_ = false;

					//* もしすでに同じ座標にあるヒエラルキーがある場合、配置せず引き返す（重複する配置の対策）
					for (const Hierarchy<Model>& model : hierarchyPanel_.GetHierarchyModels()) {

						/* 選択中以外の、アセット以外のIDのヒエラルキーはとばす */
						if (model.id_ != assetsPanel_.selectedHierarchyResource_.pathId_)
							continue;

						if (
							subPosition.x == model.transform_.get()->translation_.x &&
							subPosition.y == model.transform_.get()->translation_.y &&
							subPosition.z == model.transform_.get()->translation_.z) {
							break_ = true;
							break;
						}
					}

					if (break_) break;// breakの場合ここで

					///* モデルのヒエラルキ追加 */
					Hierarchy<Model> h;

					h.type_ = HierarchyType::kModel; // タイプ
					h.id_ = assetsPanel_.selectedHierarchyResource_.pathId_;
					h.name_ = assetsPanel_.selectedHierarchyResource_.name_;

					// WorldTransform座標決定
					h.transform_ = std::make_unique<WorldTransform>();

					h.transform_->Initialize();
					h.transform_->translation_ = { subPosition.x,subPosition.y,subPosition.z };

					WorldTransform* world = static_cast<WorldTransform*>(h.transform_.get());
					
					h.isView_ = true;

					// 位置更新
					world->UpdateMatrix();


					/* ヒエラルキ-モデル - 追加 */
					hierarchyPanel_.AddHierarchyModel(h);

				}

				break;

			}
		}


		/* ImGuiの枠にかさならないよう右クリックを押された際の処理 */
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {


			// スクリーンの大きさとスクリーン座標からから、レイ生成
			Camera::Ray ray = camera_.ScreenPointToRay(Vector2{ mouse.x,mouse.y }, width, height);

			float distance = 0.0f;


			//* モード別の処理　*//
			switch (currentModelHierarchyMode_) {

			//* 選択モード
			case ModelHierarchyMode::kSelectMode:

				if (!selectedModelWorldTransform_) break;

				hierarchyPanel_.DeleteHierarchyModel();
				selectedModelWorldTransform_ = nullptr;

				break;
			//* 配置モード
			case ModelHierarchyMode::kCreateMode:

				if (assetsPanel_.selectedHierarchyResource_.pathId_.empty()) break;

				// グリッドのplaneと判定
				std::optional<Vector3> hit = camera_.IntersectRayPlaneArea(ray, grid.plane_);

				// 当たったら
				if (hit.has_value()) {

					// すべてのモデル
					for (int i = 0; i < hierarchyPanel_.GetHierarchyModels().size(); i++)
					{
					
						/* 選択中以外の、アセット以外のIDのヒエラルキーは選択しない */
						if (hierarchyPanel_.GetHierarchyModels().at(i).id_ != assetsPanel_.selectedHierarchyResource_.pathId_)
							continue;

						// ヒエラルキからワールドのポインタ取得！
						WorldTransform* world = static_cast<WorldTransform*>(hierarchyPanel_.GetHierarchyModels().at(i).transform_.get());

						// だいたいの中心
						float size = 1.0f;

						AABB aabb;
						aabb.min = Vector3{ -size, -size, -size } + world->translation_;
						aabb.max = Vector3{ size,  size,  size } + world->translation_;

						// モデルのAABBと判定
						if (camera_.IntersectRayAABB(ray, aabb, distance)) {

							/* 当たったヒエラルキモデルを削除 */
							hierarchyPanel_.DeleteHierarchyModel(i);

							break;
						}
					}
				}

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

	ImGuizmo::SetRect(0.0f, 0.0f, w, h);


	//ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	/*カメラ視点の回転 */

	/* ホイール操作 */
	if (Input::GetInstance()->GetMouseWheel() != 0) {
		if (selectedModelWorldTransform_) {
			
			selectedModelLength_ += static_cast<float>(Input::GetInstance()->GetMouseWheel()) * 0.075f;

			// 近いの遠いの防止
			selectedModelLength_ = std::clamp(selectedModelLength_, 10.0f, 100.0f);
			
			//  また例の処理
			//

			Vector3 targetPos = selectedModelWorldTransform_->translation_;

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

	/* ギズモによるカメラ回転 */
	if (selectedModelWorldTransform_) {

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
	}
	else {

		/* 非選択時は固定値８．０ｆ */
		Matrix4x4 viewMatrix = camera_.GetViewMatrix();

		ImGuizmo::ViewManipulate(
			&viewMatrix.m[0][0],
			8.0f,
			ImVec2(ImGui::GetIO().DisplaySize.x - 256.0f, 0.0f),
			ImVec2(256.0f, 256.0f),
			0x10101010
		);

		// 変更されたView行列をそのまま反映
		camera_.SetViewMatrix(viewMatrix);
	}


	if (isEditorMode_) {

		// 選択中のモデルがあるときだけ操作
		if (selectedModelWorldTransform_) {

			// 転置なしでそのまま渡す
			Matrix4x4 viewMat = camera_.GetViewMatrix();
			Matrix4x4 projMat = camera_.GetProjectionMatrix();
			Matrix4x4 worldMat = selectedModelWorldTransform_->matWorld_;

			bool isManipulated = ImGuizmo::Manipulate(
				&viewMat.m[0][0],
				&projMat.m[0][0],
				operation_,
				ImGuizmo::WORLD,
				&worldMat.m[0][0]
			);

			/* もしスナップ中なら */
			if (grid.isSnap_) {

				Vector3 translation = {
					worldMat.m[3][0],
					worldMat.m[3][1],
					worldMat.m[3][2],
				};

				selectedModelWorldTransform_->translation_ = Grid::snapGrid(translation);

			}

			if (isManipulated || ImGuizmo::IsUsing()) {

				switch (operation_) {
				case ImGuizmo::TRANSLATE:
					selectedModelWorldTransform_->translation_.x = worldMat.m[3][0];
					selectedModelWorldTransform_->translation_.y = worldMat.m[3][1];
					selectedModelWorldTransform_->translation_.z = worldMat.m[3][2];
					break;
				case ImGuizmo::ROTATE:
					selectedModelWorldTransform_->rotation_ = ExtractRotation(worldMat, selectedModelWorldTransform_->scale_);
					break;
				case ImGuizmo::SCALE:
					selectedModelWorldTransform_->scale_ = ExtractScale(worldMat);
					break;

				}


				selectedModelWorldTransform_->UpdateMatrix();
			}
		}
		else {

			// 非選択時はキーで移動
			FreeMoveCamera();
		}
	}
#endif

	// もしデバッグモードであれば、デバッグカメラのviewをメインカメラにコピー
	if (isEditorMode_ && Scene::GetSceneCamera() != nullptr) {
		Scene::GetSceneCamera()->SetCamera(camera_);
	}
}

// エディター描画
void Editor::Draw() {

	if (isEditorMode_) {

		grid.Update();

		assetsPanel_.Draw();

		if (selectedModelWorldTransform_) {

			// 選択のAABB
			WorldTransform* world = selectedModelWorldTransform_;

			// だいたいの中心サイズ
			float size = 1.0f;

			AABB aabb;
			aabb.min = Vector3{ -size, -size, -size } + world->translation_;
			aabb.max = Vector3{ size,  size,  size } + world->translation_;

			DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });

		}

		// グリッドの線追加
		grid.AddLine();

		DebugLine::GetInstance()->Draw(camera_);
	}


	// シーンカメラが設定されてれば
	if (Scene::GetSceneCamera() != nullptr) {

		/* ヒエラルキーモデルの描画 */
		for (const Hierarchy<Model>& hierarchy : hierarchyPanel_.GetHierarchyModels()) {

			// 非表示なら描画スキップ
			if (!hierarchy.isView_) continue;

			WorldTransform* world = static_cast<WorldTransform*>(hierarchy.transform_.get());

			// マップで登録されたidと連携したモデルで描画
			if (isEditorMode_) {
				hierarchyPanel_.GethierarchyModelResourceList()[hierarchy.id_]->Draw(*world, camera_);
			}
			// エディターモードじゃなければ、シーンのカメラで描画する
			else {
				hierarchyPanel_.GethierarchyModelResourceList()[hierarchy.id_]->Draw(*world, *Scene::GetSceneCamera());
			}

		}
		

	}



	//if (isEditorMode_) {

	//	if (electedModelWorldTransform_) {

	//		AABB aabb;
	//		aabb.min = Vector3{ -0.5f, -0.5f, -0.5f } + selectedModel_->GetWorldTransform().translation_;
	//		aabb.max = Vector3{ 0.5f,  0.5f,  0.5f } + selectedModel_->GetWorldTransform().translation_;

	//		// 四角描画
	//		DebugLine::DrawAABB(aabb, { 1.0f,0,0,1.0f });

	//	}
	//}


}

void Editor::LoadScene(std::string path)
{

	currentSceneJsonPath = path;

	hierarchyPanel_.Initialize(path, grid);
}


void Editor::SelectModelUpdateView()
{

	if (selectedModelWorldTransform_ == nullptr){
		return;
	}

	// 1) カメラを、選択したモデルのワールド座標、注視点とする
	// 2) カメラのviewMatrixからワールド座標を摘出する
	// 3) 選択モデルとカメラの距離を計算（ImViewGizmoの引数に必要）
	// 4) 距離から球面座標系に必要なthita横回転とphi縦を摘出
	// 5) 球面座標としてカメラのviewにセット、thitaとｐひもImViewGizmoにあげる

	Vector3 targetPos = selectedModelWorldTransform_->translation_;

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
}

void Editor::FreeMoveCamera()
{
	/* 移動 */
	Vector3 move{ 0,0,0 };
	float speed = 0.4f;

	if (Input::GetInstance()->PushKey(DIK_W)) {
		move.z += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		move.z += -speed;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		move.x += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		move.x += -speed;
	}
	if (Input::GetInstance()->PushKey(DIK_E)) {
		move.y += speed;
	}
	if (Input::GetInstance()->PushKey(DIK_Q)) {
		move.y -= speed;
	}

	// View行列からカメラのワールド行列を取得
	Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

	// カメラの位置を増減
	cameraWorld.m[3][0] += move.x;
	cameraWorld.m[3][1] += move.y;
	cameraWorld.m[3][2] += move.z;

	// ワールド行列からView行列に戻す
	camera_.SetViewMatrix(Inverse(cameraWorld));

}

void Editor::LightingManagerTab(bool& show) {

#ifdef USE_IMGUI

	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("Lighting-Manager", &show);


	/*
		シーン固定の光源を登録する
	*/

	if (ImGui::Button("Add_DirectionalLight")) {

		DirectionalLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}
	if (ImGui::Button("Add_SpotLight")) {

		SpotLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}
	if (ImGui::Button("Add_PointLight")) {

		PointLightParam param;
		LightingManager::GetInstance()->AddLight(param,true);

	}


	// === DirectionalLight
	if (ImGui::TreeNode("DirectionalLight")) {

		// 一覧表示
		auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

		

		for (int i = 0; i < param.count; i++)
		{
			ImGui::PushID(i);

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;


			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;

			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

			// シーンの固定の光源か否か
			bool isSceneLight = LightingManager::GetInstance()->GetIsSceneLight(&param.lights[i]);

			// もし違うなら表示しない
			//if (!isSceneLight) continue;
			std::string t = std::string("isSceneLight: ") + (isSceneLight ? "true" : "false");
			ImGui::Text("%s", t.c_str());


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

	ImGui::PopStyleColor();
#endif
}

void Editor::AssetsTab(bool& show)
{

}



void Editor::EffectEditorTimeLine()
{

#ifdef USE_IMGUI




	/* 色 */
	ImGui::PushStyleColor(
		ImGuiCol_WindowBg,
		ImVec4(0.10f, 0.14f, 0.20f, 1.0f)
	);

	ImGui::Begin("EffectView");

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

	// iro
	ImGui::PopStyleColor();
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

float Grid::gridWidth_  = 1.0f;
float Grid::gridHeight_ = 1.0f;
float Grid::gridDepth_  = 1.0f;

void Grid::Update()
{

	// オイラー角->回転行列->ベクトル変換
	rotationMatrix = MakeRotationMatrix(rotation_);


	// 初期状態の法線（水平な平面の上方向）
	Vector3 normal = { 0.0f, 1.0f, 0.0f };


	// 判定用のplaneもグリッドに同期させる

	plane_.width = gridWidth_ *  static_cast<float>(horizontal_);
	plane_.height = gridHeight_ * static_cast<float>(vertical_);


	// グリッドの中心（ローカル座標）
	Vector3 localCenter = {
		gridWidth_ * horizontal_ * 0.5f,
		0.0f,
		gridHeight_ * vertical_ * 0.5f
	};

	Vector3 offset;
	//if (isCenter_) {

	//		offset = {
	//			-gridWidth_ / 2.0f,
	//			-gridHeight_ / 2.0f,
	//			0
	//		};

	//}

	// 回転させてワールド座標へ
		plane_.point =
			TransformNormal(localCenter, rotationMatrix) + (translation_ + offset);

	// 法線を回転させる
	plane_.normal = Normalize(
		TransformNormal(normal, rotationMatrix)
	);


	plane_.width = gridWidth_ * static_cast<float>(horizontal_);
	plane_.height = gridHeight_ * static_cast<float>(vertical_);

}

void Grid::AddLine()
{
	
	if (!isSnap_) return;

	/* 升目描画 */
		const Vector4 color = { 0.0f, 1.0f, 0.0f, 1.0f };

		
		Vector3 offset = {
			0.0f,
			0.0f,
			0.0f
		};
		//if (isCenter_) {
		//	offset = {
		//		-gridWidth_ / 2.0f,
		//		
		//		0,-gridHeight_ / 2.0f,
		//	};
		//}

		// ローカル座標からワールド座標へ変換するラムÐ式
		auto TransformGridPoint = [&](const Vector3& point) {

			Vector3 local = point + offset;

			// ローカル座標を回転させる
			return TransformNormal(local, rotationMatrix) + translation_;
			};

		// X方向の線
		for (int x = 0; x <= horizontal_; ++x) {
			float posX = x * gridWidth_;

			Vector3 start = { posX, 0.0f, 0.0f };
			Vector3 end = { posX, 0.0f, vertical_ * gridHeight_ };

			DebugLine::AddLine(
				TransformGridPoint(start),
				TransformGridPoint(end),
				color
			);
		}

		// Z方向の線
		for (int z = 0; z <= vertical_; ++z) {
			float posZ = z * gridHeight_;

			Vector3 start = { 0.0f, 0.0f, posZ };
			Vector3 end = { horizontal_ * gridWidth_, 0.0f, posZ };

			DebugLine::AddLine(
				TransformGridPoint(start),
				TransformGridPoint(end),
				color
			);
		}

		// 判定用-平面描画　傾きはGridの傾きを回転行列として使う
		DebugLine::DrawPlane(plane_, rotation_, { 1.0f,0,0,1.0f });


}

void Grid::ImGuiView()
{
#ifdef USE_IMGUI

	ImGui::DragInt("Grid-Vertical", &vertical_,1,1,500);
	ImGui::DragInt("Grid-Horizontal", &horizontal_,1,1,500);

	ImGui::DragFloat("Grid-width", &gridWidth_, 0.1f, 1.0f, 500.0f);
	ImGui::DragFloat("Grid-height", &gridHeight_, 0.1f, 1.0f, 500.0f);
	ImGui::DragFloat("Grid-Depth", &gridDepth_, 0.1f, 1.0f, 500.0f);

	ImGui::DragFloat3("Grid Translation", &translation_.x, 0.1f);
	ImGui::DragFloat3("plane point", &plane_.point.x, 0.1f);
	ImGui::Separator();

	ImGui::Checkbox("isSnap", &isSnap_);
	ImGui::Checkbox("isCenter", &isCenter_);

	ImGui::SeparatorText("RotateGrid");

	if (ImGui::Button("x")) {
		rotation_.x -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

	// 横に
	if (ImGui::Button("y")) {
		rotation_.y -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

	// 横に
	if (ImGui::Button("z")) {
		rotation_.z -= std::numbers::pi_v<float> / 2.0f;
	}

	ImGui::SameLine();

#endif
}
Vector3  Grid::translation_ = {0,0,0};
bool Grid::isCenter_ = false;

Vector3  Grid::snapGrid(Vector3 v) {

	Vector3 result;
	Vector3 offset;

	if (isCenter_) {
		offset = {
			-gridWidth_ / 2.0f,
			-gridHeight_ / 2.0f,
			-gridHeight_ / 2.0f
		};
	};

	result.x = std::floor(v.x / gridWidth_) * gridWidth_ + gridWidth_ / 2.0f,
	result.y = std::floor(v.y / gridHeight_) * gridHeight_ + gridHeight_ / 2.0f;
	result.z = std::floor(v.z / gridDepth_) * gridDepth_ + gridDepth_ / 2.0f;

	result = result + offset;

	// マスの大きさで割って配置
	return result;

}
bool Editor::ToggleSwitch(const char* label, bool* value)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight() - 0.2f;
	float width = height * 2.0f;
	float radius = height * 0.5f;

	ImGui::InvisibleButton(label, ImVec2(width, height));

	// クリックした瞬間に状態を切り替える
	bool clicked = ImGui::IsItemClicked();

	if (clicked)
	{
		*value = !*value;
	}

	///* アニメーションの進行度を保存しとく　これによりうんたら
	ImGuiStorage* storage = ImGui::GetStateStorage();
	ImGuiID id = ImGui::GetID(label);

	float t = storage->GetFloat(id, *value ? 1.0f : 0.0f);
	float target = *value ? 1.0f : 0.0f;

	// 徐々に目標値へ近づける
	float speed = 10.0f;
	t = ImLerp(t, target, ImGui::GetIO().DeltaTime * speed);

	// 目標値に十分近づいたら固定
	if (std::abs(t - target) < 0.001f)
	{
		t = target;
	}

	storage->SetFloat(id, t);

	// 背景色
	ImVec4 offColor = ImGui::IsItemHovered()
		? ImVec4(0.78f, 0.78f, 0.78f, 1.0f)
		: ImVec4(0.85f, 0.85f, 0.85f, 1.0f);

	ImVec4 onColor = ImGui::IsItemHovered()
		? ImVec4(0.64f, 0.83f, 0.34f, 1.0f)
		: ImVec4(0.56f, 0.83f, 0.26f, 1.0f);

	ImU32 bgColor = ImGui::GetColorU32(
		ImLerp(offColor, onColor, t)
	);

	drawList->AddRectFilled(
		p,
		ImVec2(p.x + width, p.y + height),
		bgColor,
		height * 0.5f
	);

	// つまみ
	drawList->AddCircleFilled(
		ImVec2(
			p.x + radius + t * (width - radius * 2.0f),
			p.y + radius
		),
		radius - 1.5f,
		IM_COL32(255, 255, 255, 255)
	);

	/* 押した瞬間を返す */
	return clicked;
}


void Editor::SaveIniFile()
{
	/* iniファイルの書き込み */

	// isEditorMode
	WritePrivateProfileStringA(
		"Editor",
		"isEditorMode",
		isEditorMode_ ? "true" : "false",
		".\\PonikoEngine.ini"
	);

	// 最後に触ったJsonパス
	WritePrivateProfileStringA(
		"Editor",
		"LastJsonPath",
		currentSceneJsonPath.c_str(),
		".\\PonikoEngine.ini"
	);

	// isCenter(Grid)
	WritePrivateProfileStringA(
		"Editor",
		"isGridCenter",
		grid.isCenter_ ? "true" : "false",
		".\\PonikoEngine.ini"
	);

	//* View行列からカメラのワールド行列を復元
	Matrix4x4 cameraWorld = Inverse(camera_.GetViewMatrix());

	// 位置設定
	camera_.GetWorldTransform().translation_ = {
		cameraWorld.m[3][0],
		cameraWorld.m[3][1],
		cameraWorld.m[3][2],
	};
	camera_.GetWorldTransform().rotation_ = ExtractRotation(cameraWorld,{1.0f,1.0f,1.0f});

	// カメラの位置情報
	SaveIniFileToVector3(camera_.GetWorldTransform().translation_, "DebugCamera", "translation");
	SaveIniFileToVector3(camera_.GetWorldTransform().rotation_, "DebugCamera", "rotation");

}
void Editor::LoadIniFile()
{

	/* iniファイルの読込 */

	char buffer[MAX_PATH]{};

	// true / false読込
	GetPrivateProfileStringA(
		"Editor",
		"IsEditorMode",
		"false",
		buffer,
		sizeof(buffer),
		".\\PonikoEngine.ini"
	);

	isEditorMode_ = (strcmp(buffer, "true") == 0);


	GetPrivateProfileStringA(
		"Editor",
		"LastJsonPath",
		"",
		buffer,
		MAX_PATH,
		".\\PonikoEngine.ini"
	);

	// 現在のシーンのパスに設定
	currentSceneJsonPath = buffer;


	// true / false読込
	GetPrivateProfileStringA(
		"Editor",
		"isGridCenter",
		"false",
		buffer,
		sizeof(buffer),
		".\\PonikoEngine.ini"
	);

	grid.isCenter_ = (strcmp(buffer, "true") == 0);


	Vector3 translation;
	Vector3 rotation;

	// カメラの位置情報
	LoadIniFileToVector3(translation, "DebugCamera", "translation");
	LoadIniFileToVector3(rotation, "DebugCamera", "rotation");

	// viewにして再設定
	Matrix4x4 world = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, rotation, translation);
	camera_.SetViewMatrix(Inverse(world));
}

// Vector3書き込み
void Editor::SaveIniFileToVector3(Vector3 v, const char* janre, const char* tag) {

	// 文字列に変換
	char buffer[256];

	// 位置
	sprintf_s(
		buffer, sizeof(buffer),
		"%f,%f,%f",
		v.x, v.y, v.z
	);
	WritePrivateProfileStringA(
		janre, tag, buffer, ".\\PonikoEngine.ini"
	);
}

// Vector3読込み
void Editor::LoadIniFileToVector3(Vector3& v, const char* janre, const char* tag){

	char buffer[256]{};

GetPrivateProfileStringA(
	janre,
	tag,
	"0,0,0",
	buffer,
	sizeof(buffer),
	".\\PonikoEngine.ini"
);

sscanf_s(
	buffer,
	"%f,%f,%f",
	&v.x,
	&v.y,
	&v.z
);

}

void Editor::DrawNotification(const char* text)
{


}

void Editor::Notification::PopText(const char* t, ImVec2 s)
{

	text = t;
	size = s;
	endSize_.x = s.x;

	isPop_ = true;

	ease.Reset();
	ease.Start(EasingVec::EASE_InOutExpo, easeTime * 60.0f, 0, endSize_.y);
}

// ここら辺ゴミコードすぎワロタ
void Editor::Notification::Draw()
{
	if (!isPop_) return;

	ease.Step();
	size.y = ease.getEasingNum();

	timer_ += Timer::GetDeltaTime() * 60.0f;

	// フェードアウト開始
	if (timer_ >= easeEndTime * 60.0f - easeTime * 60.0f) {
		isClose = true;
		ease.Reset();
		ease.Start(EasingVec::EASE_OutBack, easeTime* 60.0f, endSize_.y, 0);
	}
	
	// 終了
	if (timer_ >= easeEndTime* 60.0f) {

		isPop_ = false;
		timer_ = 0;
		size.y = 0;
		ease.Reset();

	}

	/*
		↑ｇｍ
					( ᓀ‸ᓂ)
		↓ネ申
	*/

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	// 画面サイズ
	ImVec2 displaySize = ImGui::GetIO().DisplaySize;

	// 通知サイズ・位置
	ImVec2 min = {
		(displaySize.x - size.x) * 0.5f,
		(displaySize.y - size.y) * 0.2f
	};
	ImVec2 max = {
		min.x + size.x,
		min.y + size.y
	};

	// 角丸の背景
	drawList->AddRectFilled(
		min,
		max,
		ImGui::GetColorU32(backCol_),
		12.0f
	);

	// 文字
	ImVec2 textSize = ImGui::CalcTextSize(text);

	ImVec2 textPos = {
		min.x + (size.x - textSize.x) * 0.5f,
		min.y + (size.y - textSize.y) * 0.2f + 6.1f
	};

	drawList->AddText(
		textPos,
		ImGui::GetColorU32(textCol_),
		text
	);
}
