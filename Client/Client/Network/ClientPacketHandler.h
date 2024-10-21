#pragma once

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum
{
	S_EnterGame = 2,

	S_MyPlayer = 4,
	S_AddObject = 5,

	C_ChangeState = 7,
	S_ChangeState = 8,

	C_TryMove = 11,
	S_Move = 12,

	C_TryAttack = 15,
	S_Attack = 16,

	C_TryRemoveObject = 20,
	S_RemoveObject = 21,

};

// Handler 함수들
bool Handle_Invalid(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_MyPlayer(PacketSessionRef& session, Protocol::S_MyPlayer& pkt);
bool Handle_S_AddObject(PacketSessionRef& session, Protocol::S_AddObject& pkt);
bool Handle_S_RemoveObject(PacketSessionRef& session, Protocol::S_RemoveObject& pkt);
bool Handle_S_Move(PacketSessionRef& session, Protocol::S_Move& pkt);
bool Handle_S_ChangeState(PacketSessionRef& session, Protocol::S_ChangeState& pkt);



class ClientPacketHandler
{
public:

	static void Init();

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->packetId](session, buffer, len);
	}

	// 보내는 패킷을 만드는 함수들
	static SendBufferRef MakeSendBuffer(Protocol::C_TryMove& pkt) { return MakeSendBuffer(pkt, C_TryMove); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ChangeState& pkt) { return MakeSendBuffer(pkt, C_ChangeState); }
	static SendBufferRef MakeSendBuffer(Protocol::C_TryRemoveObject& pkt) { return MakeSendBuffer(pkt, C_TryRemoveObject); }
	

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

