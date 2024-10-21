#include "pch.h"
#include "Prop.h"

Prop::Prop(int32 posX, int32 posZ)
{
	//_info.set_name("AA");
	Protocol::MoveStat* moveStat = _info.mutable_movestat();
	{
		moveStat->set_state(ObjectState::OBJECT_STATE_TYPE_IDLE);
		moveStat->set_animindex(0);
		moveStat->set_posx(GAME_START_POS_X + posX);
		moveStat->set_posy(GAME_START_POS_Y);
		moveStat->set_posz(GAME_START_POS_Z + posZ);
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
