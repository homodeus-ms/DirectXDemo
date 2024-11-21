#pragma once

class Player;
class Prop;
class Monster;
class SmallMonster;
class LargeMonster;
class Projectile;
class BoundingCube;

#include "Component.h"

enum
{
	GAME_START_POS_X = 200,
	GAME_START_POS_Y = 0,
	GAME_START_POS_Z = 200,
};

enum
{
	BOUND_DIST = 5,
};

const float ROTATE_SPEED = 1.f;
const float ANIM_FRAME_SPEED = 2.5f;

const Vec3 PLAYER_EXTENTS = { 0.5f, 1.f, 0.5f };
const Vec3 LARGE_MONSTER_EXTENTS = { 5.f, 10.f, 3.f };
const Vec3 CONTAINER_EXTENTS = { 2, 1, 3.5f };
const Vec3 TOWER_EXTENTS = { 2, 1, 2 };
const float SMALL_MONSTER_RADIUS = 1.5f;
const float SPHERE_BALL_RADIUS = 1.f;

class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject() {}
	virtual ~GameObject();

	virtual void Update();
	virtual void Attack(GameObjectRef target);

	static PlayerRef CreatePlayer();
	static PropRef CreateTower(int32 posX, int32 posZ);
	static PropRef CreateContainer(int32 posX, int32 posZ);
	static SMonsterRef CreateSmallMonster(int32 posX, int32 posZ);
	static ProjectileRef CreateSphereBall(PlayerRef owner, Vec3 startPos, Vec3 dir);

	static void SetId(GameObjectRef& object, ObjectType type);

	void AddComponent(shared_ptr<class Component> component);

	// Getter
	GameRoomRef GetGameRoom() { return _room.lock(); }
	Protocol::ObjectInfo& GetInfo() { return _info; }
	ObjectState GetState() { return _info.state(); }
	uint64 GetID() { return _info.objectid(); }
	uint32 GetLevel() { return _info.level(); }
	ObjectType GetObjectType();
	Vec3 GetWorldPos();
	float GetSpeed() { return _info.movestat().speed(); }
	
	shared_ptr<Component> GetComponent(ComponentType type);
	shared_ptr<class BaseCollider> GetCollider();
	BoundingCube GetBoundingCube();

	// Setter
	void SetRoom(GameRoomRef room) { _room = room; }
	void UpdateInfo(Protocol::C_TryMove& pkt);
	void SetInfo(Protocol::ObjectInfo& info) { _info = info; }
	void SetPos(Vec3 newPos);
	 
protected:

	Protocol::ObjectInfo _info;
	weak_ptr<GameRoom> _room;
	
	static atomic<uint64> s_id;

	array<shared_ptr<Component>, COMPONENT_COUNT> _components;
};

