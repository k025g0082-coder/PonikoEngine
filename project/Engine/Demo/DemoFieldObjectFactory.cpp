#include "DemoFieldObjectFactory.h"

#include "Scene.h"

#include "DemoPlayer.h"
#include "Model.h"

void DemoFieldObjectFactory::Initialize()
{
	/* 事前に必要な処理を登録 */

	Register(DemoFieldObject::BlockType::kNormal, [](const GameObjectData& data,nlohmann::json* json) {
		return std::make_unique<  DemoBlock  >( data.model, Scene::GetSceneCamera(), data, json );
	});

}
void DemoGimmickFactory::Initialize()
{
	// 
	Register(DemoGimmick::GimmickType::kSwitch, [](const GameObjectData& data, nlohmann::json* json) {
		return std::make_unique<  DemoSwitch  >( data.model, Scene::GetSceneCamera(), data, json );
		});
	
	//
	Register(DemoGimmick::GimmickType::kDoor, [](const GameObjectData& data, nlohmann::json* json) {
		return std::make_unique<  DemoDoor  >(data.model, Scene::GetSceneCamera(), data, json);
		});
	
	//
	Register(DemoGimmick::GimmickType::kGoal, [](const GameObjectData& data, nlohmann::json* json) {
		return std::make_unique<  DemoGoal  >( data.model, Scene::GetSceneCamera(), data, json );
	});
}


