#include "pch.h"
#include "SkillPacketHandler.h"
#include "ServerPacketHandler.h"
#include "GameRoom.h"
#include "GameObject.h"
#include "Player.h"

void SkillPacketHandler::HandleSkillPacket(uint64 id)
{
	PlayerRef player = GRoom->FindPlayer(id);
	
	SendSkillPacket(id);
	
}

void SkillPacketHandler::HandleSpecialSkillPacket(Protocol::C_SpecialSkill pkt)
{
	uint64 id = pkt.id();
	uint64 now = ::GetTickCount64();
	if (now - _lastSpecialSkillTime > SPECIAL_SKILL_COOL_TIME)
	{
		_lastSpecialSkillTime = now;
		SendSpecialSkillPacket(id, now);
	}
	else
	{
		return;
	}
}


void SkillPacketHandler::SendSkillPacket(uint64 id)
{
	Protocol::S_Skill sendPkt;
	sendPkt.set_id(id);

	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(sendPkt);
	GRoom->Broadcast(sendBuffer);
}

void SkillPacketHandler::SendSpecialSkillPacket(uint64 id, uint64 timeStamp)
{
	Protocol::S_SpecialSkill sendPkt;
	sendPkt.set_id(id);
	sendPkt.set_timestamp(timeStamp);
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(sendPkt);
	GRoom->Broadcast(sendBuffer);
}

