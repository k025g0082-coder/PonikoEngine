#include "SceneComponentLoader.h"

#include "LightingManager.h"

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

// ライトをセーブ
void SceneComponentLoader::LightSave() {
	if (jsonData_.empty()) {
		return;
	}

	// ===DirectionalLight
	auto& param = LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

	jsonData_["DirectionalLights"]["count"] = param.count;

	for (int i = 0; i < param.count; ++i) {

		auto& light = param.lights[i];

		jsonData_["DirectionalLights"]["lights"][i]["color"] = {
		 light.color.x,
		 light.color.y,
		 light.color.z,
		 light.color.w
		};

		jsonData_["DirectionalLights"]["lights"][i]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["DirectionalLights"]["lights"][i]["intensity"] =
			light.intensity;
	}


	// === PointLight
	auto& pParam = LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

	jsonData_["PointLights"]["count"] = pParam.count;

	for (int i = 0; i < pParam.count; ++i) {

		auto& light = pParam.lights[i];

		jsonData_["PointLights"]["lights"][i]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["PointLights"]["lights"][i]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["PointLights"]["lights"][i]["intensity"] =
			light.intensity;

		jsonData_["PointLights"]["lights"][i]["radius"] =
			light.radius;
	}


	// === SpotLight
	auto& sParam = LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

	jsonData_["SpotLights"]["count"] = sParam.count;

	for (int i = 0; i < sParam.count; ++i) {

		auto& light = sParam.lights[i];

		jsonData_["SpotLights"]["lights"][i]["color"] = {
			light.color.x,
			light.color.y,
			light.color.z,
			light.color.w
		};

		jsonData_["SpotLights"]["lights"][i]["position"] = {
			light.position.x,
			light.position.y,
			light.position.z
		};

		jsonData_["SpotLights"]["lights"][i]["intensity"] =
			light.intensity;

		jsonData_["SpotLights"]["lights"][i]["direction"] = {
			light.direction.x,
			light.direction.y,
			light.direction.z
		};

		jsonData_["SpotLights"]["lights"][i]["innerAngle"] =
			light.innerAngle;

		jsonData_["SpotLights"]["lights"][i]["outerAngle"] =
			light.outerAngle;

		jsonData_["SpotLights"]["lights"][i]["distance"] =
			light.distance;

		jsonData_["SpotLights"]["lights"][i]["falloff"] =
			light.falloff;
	}
}

// ライトをロード
void SceneComponentLoader::LightLoad() {
	if (jsonData_.empty()) {
		return;
	}

	// === DirectionalLight
	auto& dParam =
		LightingManager::GetInstance()->GetLightSets().dLight->GetDirectionalLightsParam();

	if (jsonData_.contains("DirectionalLights")) {

		auto& data = jsonData_["DirectionalLights"];

		dParam.count = data.value("count", 0);

		for (int i = 0; i < dParam.count; ++i) {

			auto& light = dParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Direction
			auto direction = json["direction"];
			light.direction = {
				direction[0],
				direction[1],
				direction[2]
			};

			light.intensity = json.value("intensity", 1.0f);
		}
	}


	// === PointLight
	auto& pParam =
		LightingManager::GetInstance()->GetLightSets().pLight->GetPointLightsParam();

	if (jsonData_.contains("PointLights")) {

		auto& data = jsonData_["PointLights"];

		pParam.count = data.value("count", 0);

		for (int i = 0; i < pParam.count; ++i) {

			auto& light = pParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Position
			auto position = json["position"];
			light.position = {
				position[0],
				position[1],
				position[2]
			};

			light.intensity = json.value("intensity", 1.0f);
			light.radius = json.value("radius", 10.0f);
		}
	}


	// === SpotLight
	auto& sParam =
		LightingManager::GetInstance()->GetLightSets().sLight->GetSpotLightsParam();

	if (jsonData_.contains("SpotLights")) {

		auto& data = jsonData_["SpotLights"];

		sParam.count = data.value("count", 0);

		for (int i = 0; i < sParam.count; ++i) {

			auto& light = sParam.lights[i];
			auto& json = data["lights"][i];

			// Color
			auto color = json["color"];
			light.color = {
				color[0],
				color[1],
				color[2],
				color[3]
			};

			// Position
			auto position = json["position"];
			light.position = {
				position[0],
				position[1],
				position[2]
			};

			// Direction
			auto direction = json["direction"];
			light.direction = {
				direction[0],
				direction[1],
				direction[2]
			};

			light.intensity = json.value("intensity", 1.0f);
			light.innerAngle = json.value("innerAngle", 15.0f);
			light.outerAngle = json.value("outerAngle", 30.0f);
			light.distance = json.value("distance", 50.0f);
			light.falloff = json.value("falloff", 1.0f);
		}
	}

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
	world.Initialize();

	world.scale_ = JsonToVector3(value["scale"]);
	world.rotation_ = JsonToVector3(value["rotation"]);
	world.translation_ = JsonToVector3(value["translation"]);

	world.UpdateMatrix();

	return world;
}