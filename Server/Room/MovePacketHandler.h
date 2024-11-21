#pragma once

class MovePacketHandler
{
public:
	
	void HandleMove(Protocol::C_TryMove pkt);

private:
	void CalculateNextPos(OUT Protocol::C_TryMove& pkt);
	void SendMovePacket(Protocol::C_TryMove& pkt);
	Vec3 GetDirNormal(const Vec3& to, const Vec3& from);
	void FillPos(Protocol::MoveStat* movestat, const Vec3& pos);
	float GetLimitAllowDiff(float speed, float rttAverage);
	Vec3 GetPosFromMoveStat(Protocol::MoveStat* movestat) { return Vec3(movestat->posx(), movestat->posy(), movestat->posz()); }

private:
	const float EMPTY_RTT_VALUE = -1.f;
	const float MOVE_PACKET_FREQUENCY = 0.2f;
	const float MOVE_POS_TOLERANCE = 0.2f;

	bool _collided = false;
};

