#include "pch.h"
#include "RttRecorder.h"
#include "GameRoom.h"
#include "Player.h"
#include "GameSession.h"
#include "ServerPacketHandler.h"

void RttRecorder::SendPing(uint64 id)
{
	PlayerRef player = GRoom->FindPlayer(id);
	ASSERT_CRASH(player != nullptr, "SendPing(), Player is nullptr");
	GameSessionRef session = player->GetSession();

	SendPing(id, session);
}

void RttRecorder::SendPing(uint64 id, GameSessionRef session)
{
	Protocol::Ping pkt;
	pkt.set_id(id);
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);

	_sendTimeCheckMap[id] = GRoom->GetTimeStamp();
	session->Send(sendBuffer);
}

void RttRecorder::Arrive(Protocol::Ping pkt)
{
	uint64 arriveTime = GRoom->GetTimeStamp();
	uint64 id = pkt.id();
	uint64 rtt;

	{
		auto findIt = _sendTimeCheckMap.find(id);
		ASSERT_CRASH(findIt != _sendTimeCheckMap.end(), "Arrive(), RTT Check Error");

		rtt = arriveTime - findIt->second;
	}

	// Update Rtt Record
	{
		auto findIt = _rttRecords.find(id);

		if (findIt == _rttRecords.end())
		{
			RttData data(rtt, 0, 1);
			_rttRecords[id] = data;
		}
		else if (GetRttRecordCount(id) < 10)
		{
			float m = _rttRecords[id].M;
			float v = _rttRecords[id].V;
			uint16 count = _rttRecords[id].count;

			_rttRecords[id] = CalculateNewDatas(m, v, count, rtt);
		}
		else
		{
			float m = _rttRecords[id].M;
			float v = _rttRecords[id].V;
			float count = _rttRecords[id].count;

			// Æ¢´Â µ¥ÀÌÅÍ ½ºÅµ
			float diff = (rtt - m) * (rtt - m);
			if (diff > (THRESHOLD * v))
			{
				return;
			}

			_rttRecords[id] = CalculateNewDatas(m, v, count, rtt);
		}

		if (GetRttRecordCount(id) == UINT16_MAX)
		{
			_rttRecords[id].count = 1;
		}
	}

	// TEMP LOG
	//RttData data = _rttRecords[id];
	//cout << "M : " << data.M << " V : " << data.V << " C : " << data.count << endl;
}

float RttRecorder::GetRtt(uint64 id)
{
	auto findIt = _rttRecords.find(id);

	if (findIt == _rttRecords.end())
		return -1;
	
	return findIt->second.M;
}

void RttRecorder::DeleteRecord(uint64 id)
{
	_rttRecords.erase(id);
}

uint16 RttRecorder::GetRttRecordCount(uint64 id)
{
	ASSERT_CRASH(_rttRecords.find(id) != _rttRecords.end(), "GetRttRecordCount(), No Record!");

	return _rttRecords[id].count;
}

RttData RttRecorder::CalculateNewDatas(float oldM, float oldV, float oldCount, float newData)
{
	float newM = (oldM * oldCount + newData) / (oldCount + 1);
	float newV = ((newData - oldM) * (newData - newM) + oldCount * oldV) / ++oldCount;

	return RttData(newM, newV, oldCount);
}

