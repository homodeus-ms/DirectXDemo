#include "pch.h"
#include "Player.h"

Player::Player()
{
	static int32 s_startX = 0;
	// ID 셋팅은 부모클래스의 Create.. 함수에서 함
	//_info.set_name("AA");
	Protocol::MoveStat* moveStat = _info.mutable_movestat();
	{
		moveStat->set_state(ObjectState::OBJECT_STATE_TYPE_IDLE);
		moveStat->set_animindex(0);
		moveStat->set_posx(GAME_START_POS_X + s_startX);
		moveStat->set_posy(GAME_START_POS_Y);
		moveStat->set_posz(GAME_START_POS_Z);
		moveStat->set_lookx(0);
		moveStat->set_looky(0);
		moveStat->set_lookz(1);
		moveStat->set_rotatex(0);
		moveStat->set_rotatey(3.14);
		moveStat->set_rotatez(0);
		moveStat->set_collided(false);
		moveStat->set_speed(3);
	}

	s_startX += 2;
}

Player::~Player()
{
}
