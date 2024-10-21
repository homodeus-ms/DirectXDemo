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
		ImGui::Begin("My Window"); // ���ο� ������ ����

		ImGui::Text("Hello, ImGui!");  // �ؽ�Ʈ ���

		ImGui::End();  // ������ ��
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

