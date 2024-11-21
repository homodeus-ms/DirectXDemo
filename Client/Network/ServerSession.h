#pragma once
#include "C:\kiwi\3DRenderer\ServerCore\Session.h"
#include "ClientPacketHandler.h"

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{

	}
	virtual void OnConnect() override
	{
		ImGui::Begin("My Window"); // 새로운 윈도우 시작

		ImGui::Text("Hello, ImGui!");  // 텍스트 출력

		ImGui::End();  // 윈도우 끝
	}
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		if (header->packetId == 5)
		{
			int a = 3;
		}

		ClientPacketHandler::HandlePacket(session, buffer, len);
	}
	virtual void OnSend(int32 len) override
	{

	}
	virtual void OnDisconnect() override
	{

	}

};

