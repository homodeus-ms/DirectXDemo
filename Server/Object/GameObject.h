#pragma once

class Player;
class Prop;
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
	BOUND_DIST = 50,
};

const float ROTATE_SPEED = 1.f;
const float ANIM_FRAME_SPEED = 2.5f;
const float PLAYER_COLLIDER_RADIUS = 1.f;
const Vec3 CONTAINER_EXTENTS = { 2, 1, 3.5f };
const Vec3 TOWER_EXTENTS = { 2, 1, 2 };

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
	static void SetId(GameObjectRef& object, ObjectType type);

	void AddComponent(shared_ptr<class Component> component);

	// Getter
	Protocol::ObjectInfo& GetInfo() { return _info; }
	uint64 GetID() { return _info.objectid(); }
	Vec3 GetWorldPos();
	ObjectType GetObjectType();
	shared_ptr<Component> GetComponent(ComponentType type);
	shared_ptr<class BaseCollider> GetCollider();
	BoundingCube GetBoundingCube();

	// Setter
	void SetRoom(GameRoomRef room) { _room = room; }
	void UpdateMovePos(MoveStat* moveStat);

protected:

	Protocol::ObjectInfo _info;
	GameRoomRef _room;
	static atomic<uint64> s_id;

	array<shared_ptr<Component>, COMPONENT_COUNT> _components;
};

