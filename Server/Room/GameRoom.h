#pragma once
#include "JobQueue.h"
#include "Octree.h"

class MovePacketHandler;
class SkillPacketHandler;
class RttRecorder;

struct Node
{
	Node(VectorInt pos, int32 dist) : _pos(pos), _dist(dist) {}
	bool operator<(const Node& other) const { return _dist < other._dist; }
	bool operator>(const Node& other) const { return _dist > other._dist; }
	VectorInt _pos;
	int32 _dist;
};

class GameRoom : public JobQueue
{
public:
	GameRoom();
	virtual ~GameRoom();

	void Init();
	void Update();

	void EnterRoom(GameSessionRef session);
	void LeaveRoom(GameSessionRef session);
	
	uint64 GetTimeStamp();
	
	// 받은 패킷 처리 함수들
	void ChangeState(Protocol::C_ChangeState pkt);
	void TryMove(Protocol::C_TryMove pkt);
	void AddObject(GameObjectRef object);
	void RemoveObject(uint64 id);
	void ChangeDir(Protocol::C_ChangeDir pkt);
	void HandleSkill(uint64 id);
	void HandleSpecialSkill(Protocol::C_SpecialSkill pkt);
	void HandleCreateProjectile(Protocol::C_CreateProjectile pkt);

	void Broadcast(SendBufferRef& sendBuffer);

	GameRoomRef GetRoomRef() { return static_pointer_cast<GameRoom>(shared_from_this()); }
	GameObjectRef FindObject(uint64 id);
	PlayerRef FindPlayer(uint64 id) { return _players[id]; }

	// Getter
	vector <shared_ptr<GameObject>>& GetBoundingObjects(BoundingCube cube) { return _octreeRoot->GetGameObjects(cube); }
	shared_ptr<RttRecorder> GetRttRecorder() { return _rttRecorder; }
	float GetAverageRttTime(uint64 id);

	void SendStartInfos(GameSessionRef session, uint64 startServerTime);
	void AddTrash(uint64 id);

	void CheckCollision(GameObjectRef movingObj, Vec3 moveDir, OUT bool& collided);
	void CheckCollision(ProjectileRef movingObj, OUT bool& collided, OUT GameObjectRef& target);
	
	
private:
	// Consts
	enum 
	{ 
		MAX_TOWER_COUNT = 100, 
		MAX_CONTAINER_COUNT = 100, 
		MAX_SMALL_MONSTER_COUNT = 80,
		RANDOM_MAX = 300, 
	};
	enum
	{
		WORLD_SIZE = 301,
		WORLD_SIZE_MIN_Y = 0,
		WORLD_SIZE_MAX_Y = 50,
	};

	const float EMPTY_RTT_VALUE = -1.f;
	
	

private:
	
	void SpawnTowers();
	void SpawnContainers();
	void SpawnSmallMonster();
	void SpawnLargeMonster();
	void GetSpawnPos(OUT vector<vector<int16>>& spawnPos);
	void SpawnBasic(vector<vector<int16>> spawnPos);
	int32 GetRandomPos(int32 limit = 0);

	void SendObjects(GameSessionRef session, Protocol::S_AddObject& pkt);

	template<typename T>
	void SendObjects(GameSessionRef session, unordered_map<uint64, T>& map);
	

private:

	unordered_map<uint64, PlayerRef> _players;
	unordered_map<uint64, PropRef> _props;
	unordered_map<uint64, SMonsterRef> _sMonsters;
	unordered_map<uint64, LMonsterRef> _lMonsters;
	unordered_map<uint64, ProjectileRef> _projectiles;
	vector<uint64> _trasheIDs;
	
	shared_ptr<Octree> _octreeRoot;

	// Packet Handler Components
	unique_ptr<MovePacketHandler> _movePacketHandler;
	unique_ptr<SkillPacketHandler> _skillPacketHandler;
	shared_ptr<RttRecorder> _rttRecorder;

	bool bSentInitPing = false;

};

extern GameRoomRef GRoom;

template<typename T>
inline void GameRoom::SendObjects(GameSessionRef session, unordered_map<uint64, T>& map)
{
	Protocol::S_AddObject pkt;
	int32 count = 0;

	for (auto& item : map)
	{
		++count;
		Protocol::ObjectInfo* info = pkt.add_objects();
		*info = item.second->GetInfo();
		
		if (count > 50)
		{
			count = 0;
			SendObjects(session, pkt);
			pkt.clear_objects();
		}
	}

	if (pkt.objects_size() > 0)
		SendObjects(session, pkt);
}
