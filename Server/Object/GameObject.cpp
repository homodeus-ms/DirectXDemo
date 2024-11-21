#include "pch.h"
#include "GameObject.h"
#include "GameRoom.h"
#include "ServerPacketHandler.h"
#include "Player.h"
#include "Prop.h"
#include "SmallMonster.h"
#include "LargeMonster.h"
#include "Projectile.h"
#include "SphereBall.h"

#include "BaseCollider.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "BoundingCube.h"


atomic<uint64> GameObject::s_id = 1;

GameObject::~GameObject()
{
	
}

void GameObject::Update()
{
	shared_ptr<Component> comp = GetComponent(ComponentType::Collider);
	shared_ptr<BaseCollider> collider = static_pointer_cast<BaseCollider>(comp);
	if (collider)
	{
		collider->SetNewCenter(GetWorldPos());
	}
}

void GameObject::Attack(GameObjectRef target)
{
}

PlayerRef GameObject::CreatePlayer()
{
	PlayerRef player = make_shared<Player>();
	GameObjectRef object = static_pointer_cast<GameObject>(player);
	SetId(object, ObjectType::OBJECT_TYPE_PLAYER);

	auto collider = make_shared<OBBBoxCollider>(player);
	collider->GetBoundingBox().Center = player->GetWorldPos();
	collider->GetBoundingBox().Extents = PLAYER_EXTENTS;

	player->AddComponent(collider);
	
	return player;
}

PropRef GameObject::CreateTower(int32 posX, int32 posZ)
{
	shared_ptr<Prop> prop = make_shared<Prop>(posX, posZ);
	GameObjectRef object = static_pointer_cast<GameObject>(prop);
	SetId(object, ObjectType::OBJECT_TYPE_PROP_TOWER);

	auto collider = make_shared<OBBBoxCollider>(prop);
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

	auto collider = make_shared<OBBBoxCollider>(prop);
	collider->GetBoundingBox().Center = prop->GetWorldPos();
	collider->GetBoundingBox().Extents = CONTAINER_EXTENTS;
	
	prop->AddComponent(collider);

	return prop;
}

SMonsterRef GameObject::CreateSmallMonster(int32 posX, int32 posZ)
{
	SMonsterRef monster = make_shared<SmallMonster>(posX, posZ);
	GameObjectRef object = static_pointer_cast<GameObject>(monster);
	SetId(object, ObjectType::OBJECT_TYPE_SMALL_MONSTER);

	auto collider = make_shared<SphereCollider>(monster);
	collider->GetBoundingSphere().Center = object->GetWorldPos();
	collider->GetBoundingSphere().Radius = SMALL_MONSTER_RADIUS;
	object->AddComponent(collider);

	return monster;
}

ProjectileRef GameObject::CreateSphereBall(PlayerRef owner, Vec3 startPos, Vec3 dir)
{
	GameObjectRef obj = make_shared<SphereBall>();
	ProjectileRef projectile = static_pointer_cast<Projectile>(obj);

	SetId(obj, ObjectType::OBJECT_TYPE_PROJECTILE);
	
	projectile->SetProjectileInfo(owner, startPos, dir);

	auto collider = make_shared<SphereCollider>(obj);
	collider->GetBoundingSphere().Center = obj->GetWorldPos();
	collider->GetBoundingSphere().Radius = SPHERE_BALL_RADIUS;
	obj->AddComponent(collider);

	return projectile;
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
	const MoveStat& stat = _info.movestat();
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
	ASSERT_CRASH(index < COMPONENT_COUNT, "GameObject::105, index < Component Count in GetComponent()");
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
	BoundingCube cube({ pos.x - BOUND_DIST, pos.y, pos.z - BOUND_DIST },
		{ pos.x + BOUND_DIST, pos.y + BOUND_DIST, pos.z + BOUND_DIST });

	return cube;
}

void GameObject::UpdateInfo(Protocol::C_TryMove& pkt)
{
	MoveStat* origin = _info.mutable_movestat();
	_info.set_state(pkt.state());
	origin->CopyFrom(pkt.movestat());

	auto collider = GetCollider();
	collider->SetNewCenter({ origin->posx(), origin->posy(), origin->posz() });
	
}

void GameObject::SetPos(Vec3 newPos)
{
	_info.mutable_movestat()->set_posx(newPos.x);
	_info.mutable_movestat()->set_posy(newPos.y);
	_info.mutable_movestat()->set_posz(newPos.z);
}

