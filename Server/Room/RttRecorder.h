#pragma once

struct RttData
{
	RttData() {}
	RttData(float m, float v, uint16 count) : M(m), V(v), count(count) {}
	float M;
	float V;
	uint16 count;
};

class RttRecorder
{
public:
	
	void SendPing(uint64 id);
	void SendPing(uint64, GameSessionRef session);
	void Arrive(Protocol::Ping pkt);
	float GetRtt(uint64 id);
	void DeleteRecord(uint64 id);
	uint16 GetRttRecordCount(uint64 id);

	RttData CalculateNewDatas(float oldM, float oldV, float oldCount, float newData);
private:
	
	// 정규분포 90% 이내 값 (z = 1.65 , 분산으로 비교하기 때문에 제곱)
	const float THRESHOLD = 2.72f;
	uint64 _sendTime;
	uint64 _arriveTime;

	unordered_map<uint64, uint64> _sendTimeCheckMap;

	// [ID] - pair<Avarage, checkCount>
	unordered_map<uint64, RttData> _rttRecords;
};

