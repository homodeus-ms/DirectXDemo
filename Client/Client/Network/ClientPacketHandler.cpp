#include "pch.h"
#include "ClientPacketHandler.h"
#include "DevApp1.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

void ClientPacketHandler::Init()
{

	for (int32 i = 0; i < UINT16_MAX; ++i)
		GPacketHandler[i] = Handle_Invalid;

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
	GPacketHandler[S_Move] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_Move>(Handle_S_Move, session, buffer, len);
	};
	
	GPacketHandler[S_RemoveObject] = [](PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		return HandlePacket<Protocol::S_RemoveObject>(Handle_S_RemoveObject, session, buffer, len);
	};
	
}



bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// todo.. 
	return false;
}

bool Handle_S_MyPlayer(PacketSessionRef& session, Protocol::S_MyPlayer& pkt)
{
	G_DevApp->MakeMyPlayer(pkt);

	return true;
}

bool Handle_S_AddObject(PacketSessionRef& session, Protocol::S_AddObject& pkt)
{
	G_DevApp->HandleAddObject(pkt);
	
	return true;
}

bool Handle_S_RemoveObject(PacketSessionRef& session, Protocol::S_RemoveObject& pkt)
{
	return false;
}

bool Handle_S_Move(PacketSessionRef& session, Protocol::S_Move& pkt)
{
	G_DevApp->HandleS_Move(pkt);
	return true;
}


bool Handle_S_ChangeState(PacketSessionRef& session, Protocol::S_ChangeState& pkt)
{
	return false;
}

