#include "pch.h"
#include "SmallMonster.h"

SmallMonster::SmallMonster(int32 x, int32 z)
{
	_info.set_state(IDLE);

	Protocol::MoveStat* moveStat = _info.mutable_movestat();
	{
		moveStat->set_posx((float)x);
		moveStat->set_posy(SMALL_MONSTER_START_Y);
		moveStat->set_posz((float)z);
		moveStat->set_lookx(0);
		moveStat->set_looky(0);
		moveStat->set_lookz(-1);
		moveStat->set_rotatex(0);
		moveStat->set_rotatey(0);
		moveStat->set_rotatez(0);
		moveStat->set_collided(false);
		moveStat->set_speed(5);
	}
}

SmallMonster::~SmallMonster()
{
}
