#include "HierarchyNode.h"

#include "WorldTransform.h"

#include "Editor.h"
#include "AssetsPanel.h"

#include "SceneComponentLoader.h"

#include "GameObject.h"

void ModelNode::Update()
{
	// WorldTransformにキャスト・・
	if (auto* world = dynamic_cast<WorldTransform*>(transform_.get())) {

		/* AABB更新 */
		aabb_.UpdatePosition(world->translation_);

		// 更新
		world->UpdateMatrix();
	}
}

void ModelNode::Draw(Camera& camera)
{
    /* 
    【重要】もしシーン内のオブジェクトと共有していたら、描画はシーン側で行うものとして、描画をスキップする
    */
    if (gameObject_)return;

    if (!isView_)return;


    WorldTransform* world = static_cast<WorldTransform*>(transform_.get());

    // マップで登録されたidと連携したモデルで描画
    if (Editor::IsEditorMode()) {

        AssetsPanel::GetInstance()->GethierarchyModelResourceList()[modelId_]->Draw(*world, camera);

    }

    // エディターモードじゃなければ、シーンのカメラで描画する
    else {

        AssetsPanel::GetInstance()->GethierarchyModelResourceList()[modelId_]->Draw(*world, *Scene::GetSceneCamera());

    }
}

void ModelNode::ImGuiView()
{
#ifdef USE_IMGUI

    /*
        選択中のモデル画面
    */
    ImGui::Begin(name_.c_str());
    /* 
        選択中のモデルヒエラルキのAABB画面
    */

    aabb_.ImGuiView();

    if (gameObject_) {
        gameObject_->SetAABB(aabb_);// GameObjectと連携場合、AABBも同期させる
    }


    ImGui::SeparatorText("GameObject");


    // それに登録されたIMGUI表示！！

    if (gameObject_) {
        gameObject_->ImGuiView();
    }

    ImGui::End();
#endif
}



void ModelNode::ChangeHierarchyLayer(bool b)
{
    //
    HierarchyNode::ChangeHierarchyLayer(b);

    // もしゲームオブジェクトと連携してたらそっちも
    if (gameObject_) {
        gameObject_->SetView(b);
    }

}

std::unique_ptr<HierarchyNode> ModelNode::Clone() const
{

    /* ここで独自の型のコピーをつくる！！ */
    auto newNode = std::make_unique<ModelNode>();


    // 共通プロパティのコピー
    newNode->name_ = this->name_ + "_Copy";
    newNode->layerName_ = this->layerName_;
    newNode->isView_ = this->isView_;

    // 3D特有データの複製
    newNode->modelId_ = this->modelId_;
    newNode->modelResource_ = this->modelResource_;
    newNode->aabb_ = this->aabb_;

    // WorldTransform のディープコピー
    if (this->transform_) {

        auto srcWorld = dynamic_cast<WorldTransform*>(this->transform_.get());
        auto newWorld = std::make_unique<WorldTransform>();

        newWorld->Initialize();

        if (srcWorld) {

            // SRTコピー
            newWorld->translation_ = srcWorld->translation_;
            newWorld->rotation_ = srcWorld->rotation_;
            newWorld->scale_ = srcWorld->scale_;
            newWorld->UpdateMatrix();
        }

        newNode->transform_ = std::move(newWorld);
    }

    return newNode;

}



nlohmann::json ModelNode::ToJson() const
{

    /* まずはヒエラルキの基本情報を取得 */
    nlohmann::json item = HierarchyNode::ToJson();

    item["id"] = modelId_;

    item["aabb"] = SceneComponentLoader::AABBToJson(aabb_);


    item["modelColor"] = Vector4ToJson(modelColor_);


     // シーン内のカスタム設定。空じゃなけりゃ保存
     if (!json_.empty()) {
         item["propaties"] = json_;
     }

     return item;
}

void ModelNode::FromJson(const nlohmann::json& j)
{
    /* まずはヒエラルキの基本情報をJSON化 */
    HierarchyNode::FromJson(j);
    
    modelId_ = j.at("id").get<std::string>();

    if (j.contains("aabb")) {
       aabb_ = SceneComponentLoader::JsonToAABB(j.at("aabb"));
    }
    if (j.contains("modelColor")) {
        modelColor_ = JsonToVector4(j.at("modelColor"));
    }

    if (j.contains("propaties")) {
        json_ = j["propaties"];
    }

}

nlohmann::json HierarchyNode::ToJson() const
{

    nlohmann::json item;

    item["type"] = type_;
    item["name"] = name_;
    item["layerName"] = layerName_;

    // ヒエラルキのトランスフォーム情報を（強引に）ワールドのにして保存
    item["transform"] = SceneComponentLoader::WorldToJson(
        *static_cast<WorldTransform*>(transform_.get())
    );

    return item;
}

void HierarchyNode::FromJson(const nlohmann::json& j)
{

    type_ = (HierarchyType)j.at("type").get<int>();
    name_ = j.at("name").get<std::string>();
    layerName_ = j.at("layerName").get<std::string>();

    // ワールドトランスフォーム初期化　makeUniqueで生成！
    auto world = SceneComponentLoader::JsonToWorld(j.at("transform"));

    transform_ =
        std::make_unique<WorldTransform>(world);

}

// 子で実装された場合、必ず実行される
void HierarchyNode::ChangeHierarchyLayer(bool b)
{
    isView_ = b;
}
