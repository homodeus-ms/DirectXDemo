#pragma once

class Player;

class SkillPacketHandler
{
public:

	void HandleSkillPacket(uint64 id);
	void HandleSpecialSkillPacket(Protocol::C_SpecialSkill pkt);

	void SendSkillPacket(uint64 id);
	void SendSpecialSkillPacket(uint64 id, uint64 timeStamp);

private:
	

private:
	uint64 SPECIAL_SKILL_COOL_TIME = 3000;

	uint64 _lastSpecialSkillTime = 0;
};

