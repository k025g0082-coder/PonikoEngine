#include "FieldExample.h"
#include "Scene.h"

#include "Editor.h"
#include "HierarchyPanel.h"

#include "GameObject.h"
#include "DemoFieldObjectFactory.h"

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

	// ロード
	FieldLoad();
}

void FieldExample::Update(Input * input)
{
	input;
	
	camera_.Update();


	player_->Update();


	/* 更新 */
	for (const std::unique_ptr<DemoFieldObject>& obj : blocks_) {
		obj->Update();
	}

	for (const std::unique_ptr<DemoGimmick>& gmk : gimmicks_) {
		gmk->Update();
	}

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

	
	//  ヒエラルキーのレイヤーを仕分けられるようになるまで一旦リターン..
	return;



	//// Editorからレイヤーでしわけられた（名前で指定）ヒエラルキーリストをもらう（予定
	//std::vector<Hierarchy<Model>> hierarchys = Editor::GetInstance()->GetHierarchyPanel().GetHierarchyModels();
	//
	///* エディター側ー＞ゲーム側に　データを受け渡す！！！！ */
	//for (Hierarchy<Model>& h : hierarchys) {

	//	// キャスト
	//	auto* world = dynamic_cast<WorldTransform*>(h.transform_.get());

	//	GameObjectData data_;

	//	/* ヒエラルキーから情報取り出す */
	//	data_.position = world->translation_;
	//	data_.rotation = world->rotation_;
	//	data_.scale = world->scale_;


	//	/* ファクトリーに登録された、クラス生成の型である識別idを、ヒエラルキーJSONから取得 */
	//	uint32_t value = h.json_["FieldObject"]["ObjectType"].get<uint32_t>();

	//	/// それぞれのタイプに登録されたラムダ式を実行して生成！！！！
	//	blocks_.push_back(std::move(DemoFieldObjectFactory::Create((DemoFieldObject::ObjectType)value)));

	//}

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
