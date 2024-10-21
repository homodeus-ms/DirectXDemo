#include "pch.h"
#include "GameObject.h"
#include "GameRoom.h"
#include "ServerPacketHandler.h"
#include "Player.h"
#include "Prop.h"

#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "BoundingCube.h"

atomic<uint64> GameObject::s_id = 1;

GameObject::~GameObject()
{
	_room = nullptr;
}

void GameObject::Update()
{

}

void GameObject::Attack(GameObjectRef target)
{
}

PlayerRef GameObject::CreatePlayer()
{
	PlayerRef player = make_shared<Player>();
	GameObjectRef object = static_pointer_cast<GameObject>(player);
	SetId(object, ObjectType::OBJECT_TYPE_PLAYER);

	auto collider = make_shared<SphereCollider>();
	collider->GetBoundingSphere().Center = player->GetWorldPos();
	collider->SetRadius(PLAYER_COLLIDER_RADIUS);
	player->AddComponent(collider);
	
	return player;
}

PropRef GameObject::CreateTower(int32 posX, int32 posZ)
{
	shared_ptr<Prop> prop = make_shared<Prop>(posX, posZ);
	GameObjectRef object = static_pointer_cast<GameObject>(prop);
	SetId(object, ObjectType::OBJECT_TYPE_PROP_TOWER);

	auto collider = make_shared<OBBBoxCollider>();
	collider->GetBoundingBox().Center = prop->GetWorldPos();
	collider->GetBoundingBox().Extents = TOWER_EXTENTS;
	prop->AddComponent(collider);
	
	return prop;
}
PropRef GameObject::CreateContainer(int32 posX, int32 posZ)
{
	shared_ptr<Prop> prop = make_shared<Prop>(posX, posZ);
	GameObjectRef object = static_pointer_cast<GameObject>(prop);
	SetId(object, ObjectType::OBJECT_TYPE_PROP_CONTAINER);

	auto collider = make_shared<AABBBoxCollider>();
	collider->GetBoundingBox().Center = prop->GetWorldPos();
	collider->GetBoundingBox().Extents = CONTAINER_EXTENTS;
	prop->AddComponent(collider);

	return prop;
}

void GameObject::SetId(GameObjectRef& object, ObjectType type)
{
	Protocol::ObjectInfo& info = object->GetInfo();
	int64 id = s_id++;

	int64 flag = static_cast<int64>(type);
	flag <<= 32;
	id |= flag;

	info.set_objectid(id);
}
void GameObject::AddComponent(shared_ptr<Component> component)
{
	component->SetGameObject(shared_from_this());
	ComponentType type = component->GetType();
	uint8 idx = static_cast<uint8>(component->GetType());

	_components[idx] = component;
}
Vec3 GameObject::GetWorldPos()
{
	MoveStat stat = _info.movestat();
	Vec3 pos = { stat.posx(), stat.posy(), stat.posz() };
	return pos;
}
ObjectType GameObject::GetObjectType()
{
	int64 id = _info.objectid();
	id >>= 32;
	return static_cast<ObjectType>(id);
}

shared_ptr<Component> GameObject::GetComponent(ComponentType type)
{
	uint8 index = static_cast<uint8>(type);
	ASSERT_CRASH(index < COMPONENT_COUNT);
	return _components[index];
}

shared_ptr<BaseCollider> GameObject::GetCollider()
{
	shared_ptr<Component> component = GetComponent(ComponentType::Collider);
	return static_pointer_cast<BaseCollider>(component);
}

BoundingCube GameObject::GetBoundingCube()
{
	Vec3 pos = GetWorldPos();
	BoundingCube cube({ pos.x - BOUND_DIST, pos.y - BOUND_DIST, pos.z - BOUND_DIST },
		{ pos.x + BOUND_DIST, pos.y + BOUND_DIST, pos.z + BOUND_DIST });

	return cube;
}

void GameObject::UpdateMovePos(MoveStat* moveStat)
{
	MoveStat* origin = _info.mutable_movestat();
	*origin = *moveStat;

	auto collider = GetCollider();
	collider->SetNewCenter({ moveStat->posx(), moveStat->posy(), moveStat->posz() });
	
}

