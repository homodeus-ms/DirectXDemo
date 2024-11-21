#include "pch.h"
#include <iomanip>
#include "GameRoom.h"
#include "Session.h"
#include "GameSession.h"
#include "ServerPacketHandler.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "SmallMonster.h"
#include "LargeMonster.h"
#include "Projectile.h"
#include "Prop.h"
#include "BoundingCube.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "MovePacketHandler.h"
#include "SkillPacketHandler.h"
#include "RttRecorder.h"

GameRoomRef GRoom = make_shared<GameRoom>();

GameRoom::GameRoom()
{
	_movePacketHandler = make_unique<MovePacketHandler>();
	_skillPacketHandler = make_unique<SkillPacketHandler>();
	_rttRecorder = make_shared<RttRecorder>();
}

GameRoom::~GameRoom()
{
	
}

void GameRoom::Init()
{
	Vec3 rootCubeCenter = { 0, 0, 0 };
	Vec3 rootCubeExtents = { WORLD_SIZE, WORLD_SIZE_MAX_Y, WORLD_SIZE };
	BoundingCube cube(rootCubeCenter, rootCubeExtents);
	_octreeRoot = make_shared<Octree>(cube);

	vector<vector<int16>> spawnPos(RANDOM_MAX);
	for (int32 i = 0; i < RANDOM_MAX; ++i)
	{
		vector<int16> v(RANDOM_MAX);
		spawnPos[i] = v;
	}

	GetSpawnPos(spawnPos);
	SpawnBasic(spawnPos);
	SpawnSmallMonster();
	
}

void GameRoom::Update()
{
	for (auto& item : _players)
	{
		item.second->Update();
	}
	for (auto& item : _projectiles)
	{
		item.second->Update();
	}
	for (uint64 id : _trasheIDs)
		RemoveObject(id);
	
	_trasheIDs.clear();
}

void GameRoom::EnterRoom(GameSessionRef session)
{
	PlayerRef player = GameObject::CreatePlayer();

	session->SetGameRoom(GetRoomRef());
	session->SetSessionPlayer(player);
	player->SetSession(session);

	// 처음 들어오면 Collider들의 Extents 정보들을 보내줌
	SendStartInfos(session, GetTimeStamp());

	// 생성된 플레이어 정보를 클라이언트로 전송함 (MyPlayer)
	{
		Protocol::S_MyPlayer pkt;
		*pkt.mutable_info() = player->GetInfo();
		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		session->Send(sendBuffer);
	}

	// 생성된 클라이언트에게 룸의 오브젝트 정보들을 전송해줌
	{
		SendObjects(session, _players);
		SendObjects(session, _props);
		SendObjects(session, _sMonsters);
		
	}

	AddObject(player);
}

void GameRoom::LeaveRoom(GameSessionRef session)
{
	PlayerRef player = session->GetSessionPlayer();
	
	// TEMP
	cout << "Player(" << player->GetID() << ")" << "is Out!" << endl;

	RemoveObject(player->GetID());
}

uint64 GameRoom::GetTimeStamp()
{
	return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}


void GameRoom::ChangeState(Protocol::C_ChangeState pkt)
{
	uint64 id = pkt.id();
	ObjectState state = pkt.state();
	GameObjectRef object = FindObject(id);

	if (object->GetState() != state)
	{
		object->GetInfo().set_state(state);
		Protocol::S_ChangeState sendPkt;
		sendPkt.set_id(id);
		sendPkt.set_state(state);
		
		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(sendPkt);
		Broadcast(sendBuffer);
	}

	_rttRecorder->SendPing(id);
}

void GameRoom::TryMove(Protocol::C_TryMove pkt)
{
	_movePacketHandler->HandleMove(pkt);
	return;
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
	case ObjectType::OBJECT_TYPE_SMALL_MONSTER:
		_sMonsters[id] = static_pointer_cast<SmallMonster>(object);
		break;
	case ObjectType::OBJECT_TYPE_LARGE_MONSTER:
		_lMonsters[id] = static_pointer_cast<LargeMonster>(object);
		break;
	case ObjectType::OBJECT_TYPE_PROJECTILE:
		_projectiles[id] = static_pointer_cast<Projectile>(object);
		break;
	default:
		return;
	}

	object->SetRoom(GetRoomRef());

	_octreeRoot->Insert(object);


	// 신규 오브젝트의 정보를 Broadcast

	if (objectType == ObjectType::OBJECT_TYPE_PROJECTILE)
	{
		ProjectileRef projectile = static_pointer_cast<Projectile>(object);
		Vec3 pos = projectile->GetWorldPos();
		Vec3 dir = projectile->GetDir();

		Protocol::S_CreateProjectile pkt;
		pkt.set_ownerid(projectile->GetOwner()->GetID());
		pkt.set_projectileid(id);
		pkt.set_type(projectile->GetProjectileType() == ProjectileType::PROJECTILE_SPHERE_BALL ? ProjectileType::PROJECTILE_SPHERE_BALL : ProjectileType::PROJECTILE_WIDE_BALL);
		Protocol::Vector3* pktPos = pkt.mutable_startpos();
		Protocol::Vector3* pktDir = pkt.mutable_dir();
		
		pktPos->set_x(pos.x);
		pktPos->set_y(pos.y);
		pktPos->set_z(pos.z);
		pktDir->set_x(dir.x);
		pktDir->set_y(dir.y);
		pktDir->set_z(dir.z);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}
	else
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

	ObjectType objectType = gameObject->GetObjectType();

	switch (objectType)
	{
	case ObjectType::OBJECT_TYPE_PLAYER:
	{
		_players.erase(id);
		_rttRecorder->DeleteRecord(id);
		break;
	}
	case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
		// intentional fall-through
	case ObjectType::OBJECT_TYPE_PROP_TOWER:
		_props.erase(id);
		break;
	case ObjectType::OBJECT_TYPE_SMALL_MONSTER:
		_sMonsters.erase(id);
		break;
	case ObjectType::OBJECT_TYPE_LARGE_MONSTER:
		_lMonsters.erase(id);
		break;
	case ObjectType::OBJECT_TYPE_PROJECTILE:
		_projectiles.erase(id);
		break;
	default:
		return;
	}

	_octreeRoot->Remove(gameObject);

	// 오브젝트 Remove 사실을 Broadcast
	{
		Protocol::S_RemoveObject pkt;
		pkt.add_ids(id);

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
		Broadcast(sendBuffer);
	}
}

void GameRoom::ChangeDir(Protocol::C_ChangeDir pkt)
{
	uint64 id = pkt.id();
	PlayerRef player = FindPlayer(id);
	ObjectInfo& info = player->GetInfo();
	info.mutable_movestat()->set_rotatey(pkt.yaw());

	{
		Protocol::S_ChangeDir sendPkt;
		sendPkt.set_id(pkt.id());
		sendPkt.set_yaw(pkt.yaw());

		SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(sendPkt);
		Broadcast(sendBuffer);
	}
}

void GameRoom::HandleSkill(uint64 id)
{
	_skillPacketHandler->HandleSkillPacket(id);
}

void GameRoom::HandleSpecialSkill(Protocol::C_SpecialSkill pkt)
{
	_skillPacketHandler->HandleSpecialSkillPacket(pkt);
}

void GameRoom::HandleCreateProjectile(Protocol::C_CreateProjectile pkt)
{
	uint64 ownerId = pkt.ownerid();
	GameObjectRef obj = FindObject(ownerId);

	if (obj == nullptr)
	{
		LOG("HandleCreateProjectile, Owner is nullptr");
		return;
	}

	ProjectileType type = pkt.type();
	PlayerRef owner = static_pointer_cast<Player>(obj);
	Vec3 pos = { pkt.startpos().x(), pkt.startpos().y(), pkt.startpos().z() };
	Vec3 dir = { pkt.dir().x(), pkt.dir().y(), pkt.dir().z() };

	switch (type)
	{
	case ProjectileType::PROJECTILE_SPHERE_BALL:
	{
		GameObjectRef projectile = GameObject::CreateSphereBall(owner, pos, dir);
		AddObject(projectile);
		break;
	}
	case ProjectileType::PROJECTILE_WIDE_BALL:
		ASSERT_CRASH(false, "HAVE NOT CREATE FUNC YET");
		break;
	default:
		ASSERT_CRASH(false, "Create Projectile switch-case default error");
		break;
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

		break;
	}
	case ObjectType::OBJECT_TYPE_PROP_TOWER:
		// intentional fall-through
	case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
	{
		auto findIt = _props.find(id);
		if (findIt != _props.end())
			return findIt->second;

		break;
	}
	case ObjectType::OBJECT_TYPE_PROJECTILE:
	{
		auto findIt = _projectiles.find(id);
		if (findIt != _projectiles.end())
			return findIt->second;

		break;
	}
	case ObjectType::OBJECT_TYPE_SMALL_MONSTER:
	{
		auto findIt = _sMonsters.find(id);
		if (findIt != _sMonsters.end())
			return findIt->second;

		break;
	}
	default:
		ASSERT_CRASH(false, "FIND OBJECT SWITCH CASE DERAULT");
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
	for (int32 i = 0; i < MAX_CONTAINER_COUNT; ++i)
	{
		int32 x = GetRandomPos();
		int32 z = GetRandomPos();

		auto prop = GameObject::CreateContainer(x, z);

		AddObject(prop);
	}
}

void GameRoom::SpawnSmallMonster()
{
	using Matrix = DirectX::SimpleMath::Matrix;

	float offsetX = 2;

	Vec3 tempStart = { 190, 0, 240 };
	
	for (int32 i = 0; i < 9; ++i)
	{
		tempStart.x += i * offsetX;
		Vec3 pos = tempStart;

		SMonsterRef monster = GameObject::CreateSmallMonster(pos.x, pos.z);

		AddObject(monster);
	}

	
}

void GameRoom::SpawnLargeMonster()
{
}

void GameRoom::GetSpawnPos(OUT vector<vector<int16>>& spawnPos)
{
	
	for (int32 i = 0; i < MAX_CONTAINER_COUNT; ++i)
	{
		int32 x, z;
		do 
		{
			x = GetRandomPos();
			z = GetRandomPos();

		} while (spawnPos[z][x] != 0);

		spawnPos[z][x] = static_cast<int16>(ObjectType::OBJECT_TYPE_PROP_CONTAINER);
	}
	for (int32 i = 0; i < MAX_TOWER_COUNT; ++i)
	{
		int32 x, z;
		do
		{
			x = GetRandomPos();
			z = GetRandomPos();
		} while (spawnPos[z][x] != 0);

		spawnPos[z][x] = static_cast<int16>(ObjectType::OBJECT_TYPE_PROP_TOWER);
	}
	for (int32 i = 0; i < MAX_SMALL_MONSTER_COUNT; ++i)
	{
		int32 x, z;
		do
		{
			x = GetRandomPos();
			z = GetRandomPos();
		} while (spawnPos[z][x] != 0);

		spawnPos[z][x] = static_cast<int16>(ObjectType::OBJECT_TYPE_SMALL_MONSTER);
	}
}

void GameRoom::SpawnBasic(vector<vector<int16>> spawnPos)
{
	int32 spawnCount = 0;

	for (int32 i = 0; i < RANDOM_MAX; ++i)
	{
		for (int32 j = 0; j < RANDOM_MAX; ++j)
		{
			if (spawnPos[i][j] == 0)
				continue;
			

			ObjectType type = static_cast<ObjectType>(spawnPos[i][j]);

			int32 posx = GAME_START_POS_X + (j - (RANDOM_MAX / 2));
			int32 posz = GAME_START_POS_Z + (i - (RANDOM_MAX / 2));

			if (posx < GAME_START_POS_X + 5 && posx > GAME_START_POS_X - 5 &&
				posz < GAME_START_POS_Z + 5 && posz > GAME_START_POS_Z - 5)
				continue;

			switch (type)
			{
			case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
			{
				AddObject(GameObject::CreateContainer(posx, posz));
				++spawnCount;
				break;
			}
			case ObjectType::OBJECT_TYPE_PROP_TOWER:
			{
				AddObject(GameObject::CreateTower(posx, posz));
				++spawnCount;
				break;
			}
			case ObjectType::OBJECT_TYPE_SMALL_MONSTER:
			{
				AddObject(GameObject::CreateSmallMonster(posx, posz));
				++spawnCount;
				break;
			}
			default:
				break;
			}
		}
	}

	cout << ++spawnCount << endl;


}

int32 GameRoom::GetRandomPos(int32 limit)
{
	int32 pos;
	int32 max = limit == 0 ? RANDOM_MAX : limit;
	pos = rand() % max;

	return pos;
}

void GameRoom::SendObjects(GameSessionRef session, Protocol::S_AddObject& pkt)
{
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	session->Send(sendBuffer);
}



float GameRoom::GetAverageRttTime(uint64 id)
{
	return _rttRecorder->GetRtt(id);
}

void GameRoom::Broadcast(SendBufferRef& sendBuffer)
{
	for (auto& item : _players)
	{
		ASSERT_CRASH(item.second->GetSession() != nullptr, "GameRoom,cpp::315, player's Session is nullptr in BroadCast()");
		item.second->GetSession()->Send(sendBuffer);
	}
}

void GameRoom::SendStartInfos(GameSessionRef session, uint64 startServerTime)
{
	Protocol::S_StartInfos pkt;
	{
		Protocol::Vector3* v = pkt.mutable_playerextents();
		v->set_x(PLAYER_EXTENTS.x);
		v->set_y(PLAYER_EXTENTS.y);
		v->set_z(PLAYER_EXTENTS.z);
	}
	{
		Protocol::Vector3* v = pkt.mutable_containerextents();
		v->set_x(CONTAINER_EXTENTS.x);
		v->set_y(CONTAINER_EXTENTS.y);
		v->set_z(CONTAINER_EXTENTS.z);
	}
	{
		Protocol::Vector3* v = pkt.mutable_towerextents();
		v->set_x(TOWER_EXTENTS.x);
		v->set_y(TOWER_EXTENTS.y);
		v->set_z(TOWER_EXTENTS.z);
	}
	{
		Protocol::Vector3* v = pkt.mutable_largemonsterextents();
		v->set_x(LARGE_MONSTER_EXTENTS.x);
		v->set_y(LARGE_MONSTER_EXTENTS.y);
		v->set_z(LARGE_MONSTER_EXTENTS.z);
	}
	{
		pkt.set_smallmonsterradius(SMALL_MONSTER_RADIUS);
	}
	

	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt);
	session->Send(sendBuffer);
}

void GameRoom::AddTrash(uint64 id)
{
	_trasheIDs.push_back(id);
}

void GameRoom::CheckCollision(GameObjectRef movingObj, Vec3 moveDir, OUT bool& collided)
{
	collided = false;

	vector<shared_ptr<BaseCollider>> colliders;
	const BoundingCube& cube = movingObj->GetBoundingCube();
	vector<shared_ptr<GameObject>>& objects = GetBoundingObjects(cube);

	for (const shared_ptr<GameObject>& object : objects)
	{
		if (object->GetCollider() == nullptr)
			continue;
		if (object->GetInfo().objectid() == movingObj->GetInfo().objectid())
			continue;

		colliders.push_back(object->GetCollider());
	}

	for (shared_ptr<BaseCollider>& other : colliders)
	{
		if (movingObj->GetCollider()->Intersects(other))
		{
			// 만약 현재 플레이어가 움직이려는 방향이 충돌체의 중심을 향하는 벡터와
			// 90도 이상 각도의 방향이라면 그냥 가게 해줌 
			Vec3 v = other->GetColliderCenter();
			v = v - movingObj->GetWorldPos();
			v.Normalize();
			if (v.Dot(moveDir) <= 0.f)
				continue;

			collided = true;
			return;
		}
	}
	collided = false;
}

void GameRoom::CheckCollision(ProjectileRef movingObj, OUT bool& collided, OUT GameObjectRef& target)
{
	collided = false;

	vector<shared_ptr<BaseCollider>> colliders;
	const BoundingCube& cube = movingObj->GetBoundingCube();
	vector<shared_ptr<GameObject>>& objects = GetBoundingObjects(cube);

	for (const shared_ptr<GameObject>& object : objects)
	{
		if (object->GetObjectType() != ObjectType::OBJECT_TYPE_SMALL_MONSTER &&
			object->GetObjectType() != ObjectType::OBJECT_TYPE_LARGE_MONSTER)
		{
			continue;
		}
		if (object->GetCollider() == nullptr)
			continue;
		
		colliders.push_back(object->GetCollider());
	}

	for (shared_ptr<BaseCollider>& other : colliders)
	{
		if (movingObj->GetCollider()->Intersects(other))
		{
			collided = true;
			target = other->GetGameObject();
			return;
		}
	}
	collided = false;
}





