#include "pch.h"
#include "Prop.h"

Prop::Prop(int32 posX, int32 posZ)
{

	_info.set_state(IDLE);

	Protocol::MoveStat* moveStat = _info.mutable_movestat();
	{
		moveStat->set_posx(static_cast<float>(posX));
		moveStat->set_posy(GAME_START_POS_Y);
		moveStat->set_posz(static_cast<float>(posZ));
		moveStat->set_lookx(0);
		moveStat->set_looky(0);
		moveStat->set_lookz(1);
		moveStat->set_rotatex(0);
		moveStat->set_rotatey(0);
		moveStat->set_rotatez(0);
		moveStat->set_collided(false);
		moveStat->set_speed(0);
	}
}

Prop::~Prop()
{
}
