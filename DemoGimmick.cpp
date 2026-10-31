#include "DemoGimmick.h"

// 基底
void DemoGimmick::Initialize()
{
	//
}

void DemoGimmick::Update()
{
	DemoFieldObject::Update();
}
void DemoGimmick::Draw()
{
	DemoFieldObject::Draw();
}


//void DemoSwitch::Initialize(Model* model, Camera* camera, const FieldData data_)
//{
//	/* 共通 */
//	DemoFieldObject::Initialize(model, camera, data_);
//	
//	// こっからIDとか取り出し
//
//	AddID(1);// ＊適当に１
//
//}


void DemoSwitch::ImGuiView()
{
	// imguiのなにか
}

void DemoGoal::ImGuiView()
{

}
