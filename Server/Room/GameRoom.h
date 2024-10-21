#pragma once
#include "JobQueue.h"
#include "Octree.h"

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
	
	
	// 받은 패킷 처리 함수들
	void ChangeState(Protocol::ObjectInfo info);
	void TryMove(Protocol::C_TryMove pkt);
	void AddObject(GameObjectRef object);
	void RemoveObject(uint64 id);

	void Broadcast(SendBufferRef& sendBuffer);

	GameRoomRef GetRoomRef() { return static_pointer_cast<GameRoom>(shared_from_this()); }
	GameObjectRef FindObject(uint64 id);

	// Getter
	vector <shared_ptr<GameObject>>& GetBoundingObjects(BoundingCube cube) { return _octreeRoot->GetGameObjects(cube); }

	void CheckCollision(PlayerRef player);

private:
	

private:
	enum { MAX_TOWER_COUNT = 100, MAX_CONTAINER_COUNT = 100, RANDOM_MAX = 100, };
	void SpawnTowers();
	void SpawnContainers();
	int32 GetRandomPos();

private:
	// For-Test
	

private:

	enum { WORLD_SIZE = 1000, };

	unordered_map<uint64, PlayerRef> _players;
	unordered_map<uint64, PropRef> _props;

	shared_ptr<Octree> _octreeRoot;

	// Collision
	bool _collided;

};

extern GameRoomRef GRoom;

