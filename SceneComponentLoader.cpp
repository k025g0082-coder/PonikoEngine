#include "SceneComponentLoader.h"

SceneComponentLoader* SceneComponentLoader::GetInstance() {
	static SceneComponentLoader instance;
	return &instance;
}

bool SceneComponentLoader::Load(const std::string& filePath)
{
	std::ifstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	try {
		file >> jsonData_;
	}
	catch (const json::parse_error&) {
		return false;
	}

	return true;
}


bool SceneComponentLoader::Save(const std::string& filePath)
{
	std::ofstream file(filePath);

	if (!file.is_open()) {
		return false;
	}

	file << jsonData_.dump(4);

	return true;
}


json Vector3ToJson(const Vector3& value)
{
	return {
		value.x,
		value.y,
		value.z
	};
}
Vector3 JsonToVector3(const json& value)
{
	return Vector3{
		value[0].get<float>(),
		value[1].get<float>(),
		value[2].get<float>()
	};
}

// WorldTransform->Jsonの形式に変換
json WorldToJson(const WorldTransform& world){
	return {
			{"scale",  Vector3ToJson(world.scale_)},
			{"rotation", Vector3ToJson(world.rotation_)},
			{"translation",Vector3ToJson(world.translation_)}
	};
}

// Json->Vector3の形式に変換
WorldTransform JsonToWorld(const json& value) {

	WorldTransform world;

	world.scale_ = JsonToVector3(value["scale"]);
	world.rotation_ = JsonToVector3(value["rotation"]);
	world.translation_ = JsonToVector3(value["translation"]);

	return world;
}