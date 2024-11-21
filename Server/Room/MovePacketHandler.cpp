#include "pch.h"
#include "MovePacketHandler.h"
#include "Player.h"
#include "GameRoom.h"
#include "ServerPacketHandler.h"

void MovePacketHandler::HandleMove(Protocol::C_TryMove pkt)
{
	uint64 id = pkt.id();
	PlayerRef player = GRoom->FindPlayer(id);
	ASSERT_CRASH(player != nullptr, "MovePacketHandler.cpp::11, player is nullptr in HandleMove()");

	if (pkt.state() != MOVE)
		return;

	if (GRoom->GetAverageRttTime(id) == EMPTY_RTT_VALUE)
	{
		player->UpdateInfo(pkt);
		SendMovePacket(pkt);
		return;
	}

	_collided = false;

	Vec3 originPos = player->GetWorldPos();
	Vec3 delayedOffset;
	CalculateNextPos(pkt);

	// 이상한 움직임이 감지 된 경우
	if (_collided)
	{
		cout << "Suspicious Move!" << endl;
		player->UpdateInfo(pkt);
		pkt.mutable_movestat()->set_collided(true);
		SendMovePacket(pkt);

		_collided = false;
		return;
	}

	// 새로운 pos로 갱신
	player->UpdateInfo(pkt);

	// 충돌 검사
	Protocol::MoveStat* movestat = pkt.mutable_movestat();
	Vec3 targetPos = GetPosFromMoveStat(movestat);
	Vec3 moveDir = GetDirNormal(targetPos, originPos);

	if (pkt.state() != ObjectState::OBJECT_STATE_TYPE_JUMP)
		GRoom->CheckCollision(player, moveDir, _collided);

	if (_collided)
	{
		cout << "Collided!" << endl;

		movestat->set_collided(true);
		pkt.set_state(IDLE);
		Vec3 dir = GetDirNormal(originPos, targetPos);
		originPos += dir * 0.2f;
		FillPos(movestat, originPos);
		player->UpdateInfo(pkt);

		_collided = false;
	}

	//cout << "SendLog : " << "{ " << movestat->posx() << ", " << movestat->posz() << " }" << endl;

	SendMovePacket(pkt);
}

void MovePacketHandler::CalculateNextPos(OUT Protocol::C_TryMove& pkt)
{
	uint64 id = pkt.id();
	PlayerRef player = GRoom->FindPlayer(id);
	ASSERT_CRASH(player != nullptr, "MovePacketHandler.cpp::75, player is nullptr in CalculateNextPos()");

	MoveStat* moveStat = pkt.mutable_movestat();
	Vec3 prevPos = player->GetWorldPos();
	Vec3 targetPos = { moveStat->posx(), moveStat->posy(), moveStat->posz() };

	float averageRttTime = GRoom->GetAverageRttTime(id) / 1000.f;
	float averageDiff = averageRttTime * player->GetSpeed();

	Vec3 dir = GetDirNormal(targetPos, prevPos);
	Vec3 offset = dir * averageDiff;

	prevPos += offset;

	float diff = (prevPos - targetPos).LengthSquared();
	float limit = GetLimitAllowDiff(player->GetSpeed(), averageRttTime);

	//cout << "DIFF: " << sqrt(diff) << " // PlayerSpeed: " << player->GetSpeed() << endl;

	// 범위를 벗어난 움직임
	if (diff > limit)
	{
		FillPos(moveStat, player->GetWorldPos());
		_collided = true;
	}
	else
	{
		targetPos += offset;    // rttTime을 계산해서 이 패킷이 도착했을 때의 클라쪽의 pos를 추정
		FillPos(moveStat, targetPos);
	}
}

void MovePacketHandler::SendMovePacket(Protocol::C_TryMove& pkt)
{
	Protocol::S_Move sendPkt;
	sendPkt.set_id(pkt.id());
	sendPkt.set_state(pkt.state());
	Protocol::MoveStat* movestat = sendPkt.mutable_movestat();
	movestat->CopyFrom(pkt.movestat());

	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(sendPkt);
	GRoom->Broadcast(sendBuffer);
}

Vec3 MovePacketHandler::GetDirNormal(const Vec3& to, const Vec3& from)
{
	Vec3 ret = to - from;
	ret.Normalize();
	return ret;
}

void MovePacketHandler::FillPos(Protocol::MoveStat* movestat, const Vec3& pos)
{
	movestat->set_posx(pos.x);
	movestat->set_posy(pos.y);
	movestat->set_posz(pos.z);
}

float MovePacketHandler::GetLimitAllowDiff(float speed, float rttAverage)
{
	float dist = 0.f;
	dist = MOVE_PACKET_FREQUENCY * speed;
	dist += rttAverage * speed + MOVE_POS_TOLERANCE;

	return dist * dist;
}
