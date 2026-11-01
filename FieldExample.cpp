#include "FieldExample.h"
#include "Scene.h"

#include "Editor.h"
#include "HierarchyPanel.h"

#include "GameObject.h"
#include "DemoFieldObjectFactory.h"

#include "ImGuiManager.h"

#include <utility>

void FieldExample::Init()
{
	camera_.Initialize();

	// カメラセット
	Scene::SetSceneCamera(&camera_);


	/* モデル（仮） */
	playerModel_ = Model::CreateOBJ("resources/Models/cube/cube.obj");
	blockModel_  = Model::CreateOBJ("resources/Models/cube/cube.obj");
	goalModel_   = Model::CreateOBJ("resources/Models/cube/cube.obj");


	// factoryら 初期化
	DemoFieldObjectFactory::Initialize();
	DemoGimmickFactory::Initialize();

	// ロード
	FieldLoad();
}

void FieldExample::Update(Input * input)
{
	input;
	
	// カメラ
	Vector3 targetPosition = player_->GetWorldTransform().translation_;
	Vector3 targetRotation = player_->GetWorldTransform().rotation_;

	Vector3 targetOffsetPos = { -6.487f,9.148f,-24.729f };
	Vector3 targetOffsetRot = { 0.300f,0.171f,-0.000f };

	// プレイヤーに追跡
	camera_.GetWorldTransform().translation_ = targetPosition + targetOffsetPos;
	camera_.GetWorldTransform().rotation_    = targetRotation + targetOffsetRot;

	camera_.Update();


	player_->Update();


	/* 更新 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {
		obj->Update();
	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {
		gmk->Update();
	}

#ifdef USE_IMGUI
	ImGui::Begin("FieldExample");

	//if (ImGui::DragFloat3("playerVelocity", &player_->velocity_.x, 0.1f)) {
	//	// なんかあれば
	//}

	if (ImGui::Button("Reload")) {
		blocks_.clear();
		gimmicks_.clear();
		FieldLoad();
	}

	ImGui::End();
#endif

}

void FieldExample::Draw()
{

	player_->Draw();


	/* 描画 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {
		obj->Draw();
	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {
		gmk->Draw();
	}

}

void FieldExample::FieldLoad()
{
	// プレイヤー
	GameObjectData data;

	player_ = std::make_unique<DemoPlayer>(playerModel_, &camera_, data);


	// Editorからレイヤーでしわけられた（名前で指定）ヒエラルキーリストをもらう（予定
	auto& hierarchys = Editor::GetInstance()->GetHierarchyPanel().GetHierarchyModelLayer("test");
	
	player_->SetStageHierarchyList(&hierarchys);// セット


	// レイヤーなかったら
	if (hierarchys.empty()) return;


	/* エディター側ー＞ゲーム側に　データを受け渡す！！！！ */
	for (Hierarchy<Model>*h : hierarchys) {

		// キャスト
		auto* world = dynamic_cast<WorldTransform*>(h->transform_.get());

		GameObjectData data_;

		// モデルももらう
		data_.model = h->object;

		// 判定用AABB
		data_.aabb_ = h->aabb_;


		/* ヒエラルキーから情報取り出す */
		data_.position = world->translation_;
		data_.rotation = world->rotation_;
		data_.scale = world->scale_;

	
		uint32_t value;

		/* ファクトリーに登録された、クラス生成の型である識別idを、ヒエラルキーJSONから取得 */
		if (!h->json_.contains("objectType")) {
			
			value = static_cast<uint32_t>(DemoFieldObject::Type::kBlock);

			// なければブロック
			h->json_["objectType"] = static_cast<uint32_t>(DemoFieldObject::Type::kBlock);
	
		}
		else {
			// タイプ名取得
			value = h->json_["objectType"].get<uint32_t>();
		}

		// タイプを記録しとく
		data_.objectID = value;

		// キー
		DemoFieldObject::Type type = static_cast<DemoFieldObject::Type>(value);



		if (!h->json_.contains("subObjectType")) {

			// なければ０番
			h->json_["subObjectType"] = 0;

		}
		else {
			// タイプ名取得
			value = h->json_["subObjectType"].get<uint32_t>();
		}

		// サブタイプを記録しとく
		data_.subObjectID = value;


		/// シーン側に,それぞれのタイプに登録された、SubIDで振り分けられたラムダ式をまとめて実行して生成！！！	

		switch (type) {

		/* ブロック */
		case DemoFieldObject::Type::kBlock:

			blocks_.push_back(std::move(DemoFieldObjectFactory::Create(static_cast<DemoFieldObject::Type>(data_.subObjectID), data_, &h->json_)));

			// ヒエラルキ側にも登録
			h->gameObject_ = blocks_.back().get();

			break;

		/* ギミック */
		case DemoFieldObject::Type::kGimmick:

			gimmicks_.push_back(std::move(DemoGimmickFactory::Create(static_cast<DemoGimmick::GimmickType>(data_.subObjectID), data_, &h->json_)));

			// ヒエラルキ側にも登録
			h->gameObject_ = gimmicks_.back().get();

			break;

		}

		
	}

}
//
//std::unique_ptr<GameObject> FieldExample::ConvertHierarchyToGameObject(std::string hierarchyLayerName,Hierarchy<Model> hierarchy)
//{
//	/* ヒエラルキーのレイヤー名と、一致しなかったら */
//	if (hierarchy.name_ != hierarchyLayerName) 
//		return nullptr;
//
//
//	// キャスト
//	auto* world = dynamic_cast<WorldTransform*>(hierarchy.transform_.get());
//
//	GameObjectData data_;
//
//	/* ヒエラルキーから情報取り出す */
//	data_.position = world->translation_;
//	data_.rotation = world->rotation_;
//	data_.scale = world->scale_;
//
//	//
//	/* モデルそのまんま渡してるから、多分いつかCloneとかするかな..でも個別に色変えたい場合とかはだけどポインタだから全部変わる・・うーｎ */
//	//
//
//	std::unique_ptr<GameObject> result;
//
//
//}
//
