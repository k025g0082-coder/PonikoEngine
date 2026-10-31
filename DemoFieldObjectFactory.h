#pragma once
#include "BaseFactory.h"

#include "DemoFieldObject.h"
#include "DemoGimmick.h"
/*
	フィールド上の登録関数
*/

// ブロック等
class DemoFieldObjectFactory : public BaseFactory<DemoFieldObject::ObjectType, DemoFieldObject>
{
	// 初期登録
	void Initialize() override;

};

// ギミック
class DemoGimmickFactory : public BaseFactory<DemoGimmick::GimmickType,DemoGimmick>
{
	// 初期登録
	void Initialize() override;

};

