#pragma once
#include "Monster.h"
class SmallMonster : public Monster
{
	using Super = Monster;

public:
	SmallMonster(int32 x, int32 z);
	virtual ~SmallMonster();

private:
	const float SMALL_MONSTER_START_Y = 1.f;
};

