#include "pch.h"
#include "ClientPacketHandler.h"
#include "DevApp1.h"
#include "NetworkLogger.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void ClientPacketHandler::Init()
{

	for (int32 i = 0; i < UINT16_MAX; ++i)
		GPacketHandler[i] = Handle_Invalid;

	GPacketHandler[S_StartInfos] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_StartInfos>(Handle_S_StartInfos, session, buffer, len);
	};

	GPacketHandler[S_MyPlayer] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_MyPlayer>(Handle_S_MyPlayer, session, buffer, len);
	};

	GPacketHandler[S_AddObject] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_AddObject>(Handle_S_AddObject, session, buffer, len);
	};

	GPacketHandler[S_ChangeState] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_ChangeState>(Handle_S_ChangeState, session, buffer, len);
	};

	GPacketHandler[S_ChangeDir] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_ChangeDir>(Handle_S_ChangeDir, session, buffer, len);
	};

	GPacketHandler[S_Move] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_Move>(Handle_S_Move, session, buffer, len);
	};
	
	GPacketHandler[S_RemoveObject] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_RemoveObject>(Handle_S_RemoveObject, session, buffer, len);
	};
	
	GPacketHandler[S_Skill] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_Skill>(Handle_S_Skill, session, buffer, len);
	};
	GPacketHandler[S_SpecialSkill] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_SpecialSkill>(Handle_S_SpecialSkill, session, buffer, len);
	};
	GPacketHandler[S_CreateProjectile] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_CreateProjectile>(Handle_S_CreateProjectile, session, buffer, len);
	};
	GPacketHandler[Ping] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::Ping>(Handle_Ping, session, buffer, len);
	};
}


bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Invalid Packet 처리 
	return false;
}

bool Handle_S_StartInfos(PacketSessionRef& session, Protocol::S_StartInfos& pkt)
{
	G_DevApp->HandleS_StartInfos(pkt);
	return true;
}

bool Handle_S_MyPlayer(PacketSessionRef& session, Protocol::S_MyPlayer& pkt)
{
	G_DevApp->MakeMyPlayer(pkt);

	return true;
}

bool Handle_S_AddObject(PacketSessionRef& session, Protocol::S_AddObject& pkt)
{
	G_DevApp->HandleAddObject(pkt);
	int32 size = pkt.objects_size();
	return true;
}

bool Handle_S_RemoveObject(PacketSessionRef& session, Protocol::S_RemoveObject& pkt)
{
	G_DevApp->HandleS_RemoveObject(pkt);
	return true;
}

bool Handle_S_Move(PacketSessionRef& session, Protocol::S_Move& pkt)
{
	G_DevApp->HandleS_Move(pkt);
	return true;
}


bool Handle_S_ChangeState(PacketSessionRef& session, Protocol::S_ChangeState& pkt)
{
	G_DevApp->HandleS_ChangeState(pkt);
	return true;
}

bool Handle_S_ChangeDir(PacketSessionRef& session, Protocol::S_ChangeDir& pkt)
{
	G_DevApp->HandleS_ChangeDir(pkt);
	return true;
}

bool Handle_S_Skill(PacketSessionRef& session, Protocol::S_Skill& pkt)
{
	G_DevApp->HandleS_Skill(pkt);
	return true;
}

bool Handle_S_SpecialSkill(PacketSessionRef& session, Protocol::S_SpecialSkill& pkt)
{
	G_DevApp->HandleS_SpecialSkill(pkt);
	return true;
}

bool Handle_S_CreateProjectile(PacketSessionRef& session, Protocol::S_CreateProjectile& pkt)
{
	G_DevApp->HandleS_CreateProjectile(pkt);
	return false;
}

bool Handle_Ping(PacketSessionRef& session, Protocol::Ping& pkt)
{
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);

	return true;
}

