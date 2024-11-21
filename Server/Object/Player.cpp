#include "pch.h"
#include "Player.h"

Player::Player()
{
	static int32 s_startX = 0;
	// ID ������ �θ�Ŭ������ Create.. �Լ����� ��
	
	_info.set_state(IDLE);
	
	Protocol::MoveStat* moveStat = _info.mutable_movestat();
	{
		moveStat->set_posx(static_cast<float>(GAME_START_POS_X + s_startX));
		moveStat->set_posy(static_cast<float>(GAME_START_POS_Y));
		moveStat->set_posz(static_cast<float>(GAME_START_POS_Z));
		moveStat->set_lookx(0);
		moveStat->set_looky(0);
		moveStat->set_lookz(1);
		moveStat->set_rotatex(0);
		moveStat->set_rotatey(3.14f);
		moveStat->set_rotatez(0);
		moveStat->set_collided(false);
		moveStat->set_speed(5);
	}

	s_startX += 2;
}

Player::~Player()
{
}
