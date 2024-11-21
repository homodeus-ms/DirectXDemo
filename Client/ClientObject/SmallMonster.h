#pragma once
#include "Monster.h"
class SmallMonster : public Monster
{
	using Super = Monster;

public:
	SmallMonster();
	virtual ~SmallMonster();

	virtual void Update() override;

private:
	const float ROTATION_SPEED = 7.f;
};

