#pragma once

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum
{
	S_EnterGame = 2,
	S_StartInfos = 3,
	
	S_MyPlayer = 5,
	S_AddObject = 6,

	C_ChangeState = 7,
	S_ChangeState = 8,

	C_ChangeDir = 9,
	S_ChangeDir = 10,

	C_TryMove = 11,
	S_Move = 12,

	C_Skill = 15,
	S_Skill = 16,
	C_SpecialSkill = 17,
	S_SpecialSkill = 18,

	C_CreateProjectile = 20,
	S_CreateProjectile = 21,

	C_TryRemoveObject = 30,
	S_RemoveObject = 31,

	C_Chat = 40,
	S_Chat = 41,

	Ping = 50,
	
};

// Handler 함수들
bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_StartInfos(PacketSessionRef& session, Protocol::S_StartInfos& pkt);
bool Handle_S_MyPlayer(PacketSessionRef& session, Protocol::S_MyPlayer& pkt);
bool Handle_S_AddObject(PacketSessionRef& session, Protocol::S_AddObject& pkt);
bool Handle_S_RemoveObject(PacketSessionRef& session, Protocol::S_RemoveObject& pkt);
bool Handle_S_Move(PacketSessionRef& session, Protocol::S_Move& pkt);
bool Handle_S_ChangeState(PacketSessionRef& session, Protocol::S_ChangeState& pkt);
bool Handle_S_ChangeDir(PacketSessionRef& session, Protocol::S_ChangeDir& pkt);
bool Handle_S_Skill(PacketSessionRef& session, Protocol::S_Skill& pkt);
bool Handle_S_SpecialSkill(PacketSessionRef& session, Protocol::S_SpecialSkill& pkt);
bool Handle_S_CreateProjectile(PacketSessionRef& session, Protocol::S_CreateProjectile& pkt);
bool Handle_Ping(PacketSessionRef& session, Protocol::Ping& pkt);



class ClientPacketHandler
{
public:

	static void Init();

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->packetId](session, buffer, len);
	}


	// Send 패킷 만드는 함수들
	static SendBufferRef MakeSendBuffer(Protocol::C_TryMove& pkt) { return MakeSendBuffer(pkt, C_TryMove); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ChangeState& pkt) { return MakeSendBuffer(pkt, C_ChangeState); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ChangeDir& pkt) { return MakeSendBuffer(pkt, C_ChangeDir); }
	static SendBufferRef MakeSendBuffer(Protocol::C_TryRemoveObject& pkt) { return MakeSendBuffer(pkt, C_TryRemoveObject); }
	static SendBufferRef MakeSendBuffer(Protocol::C_Skill& pkt) { return MakeSendBuffer(pkt, C_Skill); }
	static SendBufferRef MakeSendBuffer(Protocol::C_SpecialSkill& pkt) { return MakeSendBuffer(pkt, C_SpecialSkill); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CreateProjectile& pkt) { return MakeSendBuffer(pkt, C_CreateProjectile); }
	static SendBufferRef MakeSendBuffer(Protocol::Ping& pkt) { return MakeSendBuffer(pkt, Ping); }
	

private:


	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;
		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = make_shared<SendBuffer>(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->packetSize = packetSize;
		header->packetId = pktId;
		assert(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->SetWritePos(packetSize);

		return sendBuffer;
	}


};

