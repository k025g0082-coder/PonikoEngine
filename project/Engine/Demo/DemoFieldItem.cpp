#include "DemoFieldItem.h"

#include  <misc/cpp/imgui_stdlib.h>
#include <imgui_internal.h>

#include "FieldExample.h"

// ギミックの種類名
const char* DemoFieldItem::ItemTypeName[(int)ItemType::count] = {
	"Coin"
};


// 基底
void DemoFieldItem::Initialize()
{
	//
}

void DemoFieldItem::Update()
{
	DemoFieldObject::Update();
}
void DemoFieldItem::Draw()
{
	DemoFieldObject::Draw();
}

void DemoCoin::Initialize()
{
	// 黄色
	model_->ModelColor({ 1.0f,1.0f,0,1.0f });


	// 当たり判定無効
	isCollisionBlock_ = false;
}

//
void DemoCoin::Update()
{
	DemoFieldObject::Update();
}
void DemoCoin::Draw()
{
	DemoFieldObject::Draw();
}

void DemoCoin::onCollisionToPlayer(DemoPlayer* player, CollisionResult result)
{
	//　もし当たったら、消滅
	isDelete = true;
}