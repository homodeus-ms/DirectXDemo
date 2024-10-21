#include "pch.h"
#include "NetworkManager.h"
#include "ClientPacketHandler.h"
#include "Service.h"
#include "ThreadManager.h"
#include "ServerSession.h"
#include "Session.h"


const wstring MY_IP = L"14.56.55.201";
const wstring LOOP_BACK_IP = L"127.0.0.1";

void NetworkManager::Init()
{
	SocketUtils::Init();
	ClientPacketHandler::Init();

	_service = make_shared<ClientService>(
		NetAddress(LOOP_BACK_IP, 7777),
		make_shared<IocpCore>(),
		[=]() { return CreateSession(); },
		1);

	assert(_service->Start());
}

void NetworkManager::Update()
{
	_service->GetIocpCore()->Dispatch(0);
}

ServerSessionRef NetworkManager::CreateSession()
{
	return _session = make_shared<ServerSession>();
}

void NetworkManager::SendPacket(SendBufferRef sendBuffer)
{
	if (_session)
		_session->Send(sendBuffer);
}
