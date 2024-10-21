#include "pch.h"
#include "GameRoom.h"
#include "Session.h"
#include "GameSession.h"
#include "ServerPacketHandler.h"
#include "Player.h"
#include "Prop.h"
#include "BoundingCube.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"

GameRoomRef GRoom = make_shared<GameRoom>();

GameRoom::GameRoom()
{
	
}

GameRoom::~GameRoom()
{
	
}

void GameRoom::Init()
{
	BoundingCube cube(Vec3(-WORLD_SIZE, -WORLD_SIZE, -WORLD_SIZE), Vec3(WORLD_SIZE, WORLD_SIZE, WORLD_SIZE));
	_octreeRoot = make_shared<Octree>(cube);

	
	SpawnContainers();
	SpawnTowers();
}

void GameRoom::Update()
{
	for (auto& item : _players)
	{
		item.second->Update();
	}
	/*for (auto& item : _props)
	{
		item.second->Update();
	}*/
}

void GameRoom::EnterRoom(GameSessionRef session)
{
	PlayerRef player = GameObject::CreatePlayer();

	_octreeRoot->Insert(player);

	session->_gameRoom = GetRoomRef();
	session->_player = player;
	player->SetSession(session);

	// 생성된 플레이어 정보를 클라이언트로 전송 (S_MyPlayer)
	{
		Protocol::S_MyPlayer pkt;
		*pkt.mutable_info() = player->GetInfo();
		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		session->Send(sendBuffer);
	}
	// 생성된 클라에게 현재 룸에 존재하는 모든 오브젝트 정보를 전송해줌(S_AddObject)
	{
		Protocol::S_AddObject pkt;

		for (auto& item : _players)
		{
			Protocol::ObjectInfo* info = pkt.add_objects();
			*info = item.second->GetInfo();
		}

		for (auto& item : _props)
		{
			Protocol::ObjectInfo* info = pkt.add_objects();
			*info = item.second->GetInfo();
		}

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		session->Send(sendBuffer);
	}

	AddObject(player);
}

void GameRoom::LeaveRoom(GameSessionRef session)
{
	
}


void GameRoom::ChangeState(Protocol::ObjectInfo info)
{
	
}

void GameRoom::TryMove(Protocol::C_TryMove pkt)
{
	uint64 id = pkt.id();
	PlayerRef player = _players[id];
	ASSERT_CRASH(player != nullptr);

	MoveStat* moveStat = pkt.mutable_movestat();

	if (moveStat->state() == ObjectState::OBJECT_STATE_TYPE_IDLE)
		return;
	
	player->UpdateMovePos(moveStat);

	if (pkt.movestat().state() != ObjectState::OBJECT_STATE_TYPE_JUMP)
		CheckCollision(player);

	if (_collided)
	{
		cout << "Collided!" << endl;

		Vec3 originPos = { moveStat->posx(), moveStat->posy(), moveStat->posz() };
		Vec3 look = { moveStat->lookx(), moveStat->looky(), moveStat->lookz() };
		// Round Trip 시간 측정?
		// 임시
		const float dt = 100.f;
		Vec3 newPos = originPos + look * dt;

		Protocol::S_Move pkt;
		pkt.set_id(player->GetID());

		MoveStat* stat = pkt.mutable_movestat();
		*stat = *moveStat;
		stat->set_posx(newPos.x);
		stat->set_posy(newPos.y);
		stat->set_posz(newPos.z);
		stat->set_collided(true);

		player->UpdateMovePos(moveStat);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}
	else
	{
		Protocol::S_Move pkt;
		pkt.set_id(player->GetID());
		MoveStat* stat = pkt.mutable_movestat();
		*stat = *moveStat;

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}
	
}

void GameRoom::AddObject(GameObjectRef object)
{
	uint64 id = object->GetInfo().objectid();
	auto objectType = object->GetObjectType();
	
	switch (objectType)
	{
	case ObjectType::OBJECT_TYPE_PLAYER:
		_players[id] = static_pointer_cast<Player>(object);
		break;
	case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
		// intentional fall-through
	case ObjectType::OBJECT_TYPE_PROP_TOWER:
		_props[id] = static_pointer_cast<Prop>(object);
		break;
	
	default:
		return;
	}

	object->SetRoom(GetRoomRef());

	_octreeRoot->Insert(object);

	// 신규 오브젝트의 정보를 전송
	{
		Protocol::S_AddObject pkt;

		Protocol::ObjectInfo* info = pkt.add_objects();
		*info = object->GetInfo();

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}
}

void GameRoom::RemoveObject(uint64 id)
{
	GameObjectRef gameObject = FindObject(id);
	if (gameObject == nullptr)
		return;

	// 오브젝트 Remove사실을 전송
	{
		Protocol::S_RemoveObject pkt;
		pkt.add_ids(id);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}

}



GameObjectRef GameRoom::FindObject(uint64 id)
{
	int32 flag = id >> 32;
	ObjectType type = static_cast<ObjectType>(flag);

	switch (type)
	{
	case ObjectType::OBJECT_TYPE_PLAYER:
	{
		auto findIt = _players.find(id);
		if (findIt != _players.end())
			return findIt->second;
	}
	break;

	case ObjectType::OBJECT_TYPE_PROP_TOWER:
		// intentional fall-through
	case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
	{
		auto findIt = _props.find(id);
		if (findIt != _props.end())
			return findIt->second;
	}
	break;
	}


	return nullptr;
}

void GameRoom::SpawnTowers()
{
	for (int32 i = 0; i < MAX_TOWER_COUNT; ++i)
	{
		int32 x = GetRandomPos();
		int32 z = GetRandomPos();

		auto prop = GameObject::CreateTower(x, z);

		AddObject(prop);
	}
}

void GameRoom::SpawnContainers()
{
	for (int32 i = 0; i < MAX_TOWER_COUNT; ++i)
	{
		int32 x = GetRandomPos();
		int32 z = GetRandomPos();

		auto prop = GameObject::CreateContainer(x, z);

		AddObject(prop);
	}
}

int32 GameRoom::GetRandomPos()
{
	int32 pos;
	
	do {
		pos = rand() % RANDOM_MAX - RANDOM_MAX / 2;
	} while (pos >= -5 && pos <= 5);

	return pos;
}




void GameRoom::Broadcast(SendBufferRef& sendBuffer)
{
	for (auto& item : _players)
	{
		ASSERT_CRASH(item.second->GetSession() != nullptr);
		item.second->GetSession()->Send(sendBuffer);
	}
}

void GameRoom::CheckCollision(PlayerRef player)
{
	_collided = false;

	vector<shared_ptr<BaseCollider>> colliders;
	const BoundingCube& cube = player->GetBoundingCube();
	vector<shared_ptr<GameObject>>& objects = GetBoundingObjects(cube);

	for (const shared_ptr<GameObject>& object : objects)
	{
		if (object->GetCollider() == nullptr)
			continue;
		if (object->GetInfo().objectid() == player->GetInfo().objectid())
			continue;
		colliders.push_back(object->GetCollider());
	}

	for (auto& other : colliders)
	{
		if (player->GetCollider()->Intersects(other))
		{
			_collided = true;
			return;
		}
	}
	_collided = false;
}


