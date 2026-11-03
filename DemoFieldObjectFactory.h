#pragma once
#include "BaseFactory.h"

#include "DemoFieldObject.h"
#include "DemoGimmick.h"
/*
	フィールド上の登録関数
*/

// ブロック等
class DemoFieldObjectFactory : public BaseFactory<DemoFieldObject::BlockType, DemoFieldObject>
{
public:
	// 初期登録
	static void Initialize();



};

// ギミック
class DemoGimmickFactory : public BaseFactory<DemoGimmick::GimmickType,DemoGimmick>
{
public:
	// 初期登録
	static void Initialize();

};

