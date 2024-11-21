#include "pch.h"
#include "DevApp1.h"
#include "Scene.h"
#include "Shader.h"
#include "GameObject.h"
#include "Camera.h"
#include "CameraScript.h"
#include "ModelRenderer.h"
#include "ModelAnimation.h"
#include "Material.h"
#include "Transform.h"
#include "Model.h"
#include "ModelAnimator.h"
#include "MeshRenderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Viewport.h"
#include "SphereCollider.h"
#include "Scene.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "Terrain.h"
#include "CharacterMoveScript.h"
#include "NetworkManager.h"
#include "Player.h"
#include "MyPlayer.h"
#include "Projectile.h"
#include "ClientObject.h"
#include "Prop.h"
#include "BoundingCube.h"
#include "MyCamera.h"
#include "NetworkLogger.h"
#include "ObjectCreator.h"

// 오프라인 디버그 용 플래그
//#define OFFLINE_DEBUG 

DevApp1::DevApp1()
{
	_devScene = CUR_SCENE;
}

DevApp1::~DevApp1()
{
	if (_objectCreator != nullptr)
	{
		delete _objectCreator;
		_objectCreator = nullptr;
	}
}

void DevApp1::Init()
{
	Sleep(300);

#ifndef OFFLINE_DEBUG
	GET_SINGLE(NetworkManager)->Init();
#endif
	_shader = make_shared<Shader>(L"20. ViewportDemo.fx");
	_objectCreator = new ObjectCreator(_shader);
	

#ifdef OFFLINE_DEBUG
	Protocol::ObjectInfo info;
	info.set_objectid(1 | (1 << 32));
	info.set_state(IDLE);
	Protocol::MoveStat* moveStat = info.mutable_movestat();
	{
		moveStat->set_posx(static_cast<float>(200));
		moveStat->set_posy(static_cast<float>(0));
		moveStat->set_posz(static_cast<float>(200));
		moveStat->set_lookx(0);
		moveStat->set_looky(0);
		moveStat->set_lookz(1);
		moveStat->set_rotatex(0);
		moveStat->set_rotatey(3.14f);
		moveStat->set_rotatez(0);
		moveStat->set_collided(false);
		moveStat->set_speed(5);
	}
	auto player = _objectCreator->CreatePlayer(info, true);

	assert(player != nullptr);
	_objectCreator->CreateCamera(static_pointer_cast<MyPlayer>(player));
	_objectCreator->CreateLight();

	CUR_SCENE->Start();

#endif
	// For Test
	// CreateManyCubes();
}

void DevApp1::Update()
{

#ifndef OFFLINE_DEBUG
	GET_SINGLE(NetworkManager)->Update();
#endif
	
	// ImGui 체크
	// Frustum Calling Check

	ImGui::Begin("Frustum", nullptr);

	ImGui::Text("%s(%d), %s(%d), %s(%d), %s(%d))", 
		"TotalObject ", CUR_SCENE->_totalObjectCount, 
		"1stCulled ", CUR_SCENE->_firstCulledCount,
		"CulledObject ", CUR_SCENE->_culledCount, 
		"FPS ", TIME->GetFps());

	if (GetMyPlayer() == nullptr)
	{
		ImGui::End();
		return;
	}

	auto _camera = GetMyPlayer()->GetMyCamera();
	auto _player = GetMyPlayer();
	
	if (_camera != nullptr)
	{
		auto pt = _player->GetTransform();
		auto ct = _camera->GetTransform();
		Vec3 playerPos = pt->GetWorldPosition();
		Vec3 cameraPos = ct->GetWorldPosition();
		Vec3 cameraLocalPos = ct->GetLocalPosition();
		Vec3 angle = ct->GetLocalRotation();

		ImGui::Text("%s : %f, %f, %f", "playerPos", playerPos.x, playerPos.y, playerPos.z);
		ImGui::Text("%s : %f, %f, %f", "cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("%s : %f, %f, %f", "cameraLocalPos", cameraLocalPos.x, cameraLocalPos.y, cameraLocalPos.z);
		ImGui::Text("%s : %f, %f, %f", "cameraRotate", angle.x, angle.y, angle.z);

	} 

	ImGui::End();
}

void DevApp1::Render()
{
}

void DevApp1::MakeMyPlayer(const Protocol::S_MyPlayer& pkt)
{
	const ObjectInfo& info = pkt.info();

	shared_ptr<GameObject> player = _objectCreator->CreatePlayer(info, true);
	
	assert(player != nullptr);
	_objectCreator->CreateCamera(static_pointer_cast<MyPlayer>(player));
	_objectCreator->CreateLight();

	CUR_SCENE->Start();
}

void DevApp1::HandleS_StartInfos(const Protocol::S_StartInfos& pkt)
{
	{
		const Protocol::Vector3& v = pkt.playerextents();
		_objectCreator->PLAYER_EXTENTS.x = v.x();
		_objectCreator->PLAYER_EXTENTS.y = v.y();
		_objectCreator->PLAYER_EXTENTS.z = v.z();
	}
	{
		const Protocol::Vector3& v = pkt.containerextents();
		_objectCreator->CONTAINER_EXTENTS.x = v.x();
		_objectCreator->CONTAINER_EXTENTS.y = v.y();
		_objectCreator->CONTAINER_EXTENTS.z = v.z();
	}
	{
		const Protocol::Vector3& v = pkt.towerextents();
		_objectCreator->TOWER_EXTENTS.x = v.x();
		_objectCreator->TOWER_EXTENTS.y = v.y();
		_objectCreator->TOWER_EXTENTS.z = v.z();
	}
	{
		const Protocol::Vector3& v = pkt.largemonsterextents();
		_objectCreator->LARGE_MONSTER_EXTENTS.x = v.x();
		_objectCreator->LARGE_MONSTER_EXTENTS.y = v.y();
		_objectCreator->LARGE_MONSTER_EXTENTS.z = v.z();
	}
	{
		_objectCreator->SMALL_MONSTER_RADIUS = pkt.smallmonsterradius();
	}


}

void DevApp1::HandleAddObject(const Protocol::S_AddObject& pkt)
{
	uint64 myPlayerId = GetMyPlayerID();
	const int32 objectCount = pkt.objects_size();

	for (int32 i = 0; i < objectCount; ++i)
	{
		const Protocol::ObjectInfo& info = pkt.objects(i);
		if (myPlayerId == info.objectid())
		{
			int a = 3;
			continue;
		}

		// [앞32비트 오브젝트타입 | 뒤32비트 오브젝트 ID]
		uint64 id = info.objectid();
		ObjectType objectType = static_cast<ObjectType>((id >> 32));

		switch (objectType)
		{
		case ObjectType::OBJECT_TYPE_PLAYER:
		{
			shared_ptr<ClientObject> player = static_pointer_cast<ClientObject>(_objectCreator->CreatePlayer(info, false));
			break;
		}
		case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
		{
			CreateContainer(info);
			break;
		}
		case ObjectType::OBJECT_TYPE_PROP_TOWER:
		{
			CreateTower(info);
			break;
		}
		case ObjectType::OBJECT_TYPE_SMALL_MONSTER:
		{
			CreateSmallMonster(info);
			break;
		}
		case ObjectType::OBJECT_TYPE_LARGE_MONSTER:
		{
			CreateLargeMonster(info);
			break;
		}
		case ObjectType::OBJECT_TYPE_PROJECTILE:
		{
			CreateProjectile(info);
			break;
		}
		}
	}
	
	unordered_map<uint64, shared_ptr<GameObject>> objects = CUR_SCENE->GetObjects();
	
}

void DevApp1::HandleS_RemoveObject(const Protocol::S_RemoveObject& pkt)
{
	int32 size = pkt.ids().size();
	uint64 myPlayerID = GetMyPlayerID();

	for (uint64 id : pkt.ids())
	{
		if (id == myPlayerID)
		{
			return;
		}

		if ((id >> 32) == ObjectType::OBJECT_TYPE_PROJECTILE)
		{
			uint64 myBallID = GetMyPlayer()->GetSphereBall()->GetID();

			if (id == myBallID)
			{
				GetMyPlayer()->DisappearShootBall();
			}
			else
			{
				shared_ptr<GameObject> obj = GetTemporalObject(id);
				_devScene->RemoveTemoporalObject(obj);
				DeleteTemporalObject(id);
			}
		}
		else
		{
			_devScene->Remove(id);
		}
	}
	
}

void DevApp1::HandleS_Move(const Protocol::S_Move& pkt)
{
	uint64 id = pkt.id();
	ObjectType objectType = static_cast<ObjectType>((id >> 32));

	if (IsMyPlayer(id))
	{
		shared_ptr<MyPlayer> myPlayer = GetMyPlayer();

		if (pkt.movestat().collided() == true)
		{
			myPlayer->SetCollide(true);
			myPlayer->HandleCollided(pkt);
			myPlayer->SetState(IDLE, true);
		}
		else
		{
			myPlayer->UpdateMoveInfo(pkt);
		}
		
		return;
	}

	shared_ptr<ClientObject> obj = FindObject(id);
	obj->SetState(MOVE, false);
	obj->UpdateMoveInfo(pkt);
}

void DevApp1::HandleS_ChangeState(const Protocol::S_ChangeState& pkt)
{
	uint64 id = pkt.id();
	if (IsMyPlayer(id))
		return;

	shared_ptr<ClientObject> obj = FindObject(id);

	obj->SetState(pkt.state(), false);

	// 현재 로직에서 무브방향 키를 누르고 있다가 떼면 바로 IDLE 패킷이
	// 전송되고 이리로 돌아옴 거기서 Info의 위치와 Transform 위치에 오차가 생겼었음
	if (pkt.state() == IDLE)
	{
		obj->SyncTransformPosWithInfo();
	}
}

void DevApp1::HandleS_ChangeDir(const Protocol::S_ChangeDir& pkt)
{
	uint64 id = pkt.id();
	if (IsMyPlayer(id))
		return;

	shared_ptr<ClientObject> obj = FindObject(id);

	obj->RotateYaw(pkt.yaw());
}

void DevApp1::HandleS_Skill(const Protocol::S_Skill& pkt)
{
	uint64 id = pkt.id();
	if (IsMyPlayer(id))
		return;

	shared_ptr<ClientObject> obj = FindObject(pkt.id());
	obj->SetState(SKILL, false);
	obj->SetSkillReady();
	
}

void DevApp1::HandleS_SpecialSkill(const Protocol::S_SpecialSkill& pkt)
{
	uint64 id = pkt.id();
	if (IsMyPlayer(id))
		return;

	shared_ptr<ClientObject> obj = FindObject(pkt.id());
	obj->SetState(SPECIAL_SKILL, false);
	obj->SetSkillReady();
}

void DevApp1::HandleS_CreateProjectile(const Protocol::S_CreateProjectile& pkt)
{
	uint64 ownerId = pkt.ownerid();

	// MyPlayer의 Projectile은 발사버튼을 눌렀을 때 바로 만들어졌음
	if (ownerId == GetMyPlayerID())
	{
		GetMyPlayer()->GetSphereBall()->SetID(pkt.projectileid());
		return;
	}

	// TODO : 다른 Projectile 타입 추가
	if (pkt.type() != ProjectileType::PROJECTILE_SPHERE_BALL)
		return;

	// 발사한 주체가 아닌 다른 Player들의 화면
	shared_ptr<ClientObject> obj = FindObject(ownerId);
	shared_ptr<MyPlayer> owner = static_pointer_cast<MyPlayer>(obj);
	ASSERT_CRASH(owner != nullptr, "HandleS_CreateProjectile, owner is nullptr");

	Vec3 pos = { pkt.startpos().x(), pkt.startpos().y(), pkt.startpos().z() };
	Vec3 dir = { pkt.dir().x(), pkt.dir().y(), pkt.dir().z() };

	shared_ptr<Projectile> projectile = _objectCreator->GetSphereBall(pos, dir);
	projectile->SetID(pkt.projectileid());
	AddTemporalObject(pkt.projectileid(), projectile);
	
	SetSphereBallOn(projectile);
}

void DevApp1::CheckCollision(shared_ptr<ClientObject> player, Vec3 moveDir, OUT bool& collided)
{
	collided = false;

	vector<shared_ptr<BaseCollider>> colliders;
	const BoundingCube& cube = player->GetCollisionBoundingCube();
	vector<shared_ptr<GameObject>>& objects = CUR_SCENE->GetBoundingObjects(cube);

	for (const shared_ptr<GameObject>& obj : objects)
	{
		shared_ptr<ClientObject> object = static_pointer_cast<ClientObject>(obj);

		if (object->GetCollider() == nullptr)
			continue;
		if (object->GetInfo().objectid() == player->GetInfo().objectid())
			continue;
		colliders.push_back(object->GetCollider());
	}

	for (shared_ptr<BaseCollider>& other : colliders)
	{
		if (player->GetCollider()->Intersects(other))
		{
			// 만약 현재 플레이어가 움직이려는 방향이 충돌체의 중심으로 가는 벡터와
			// 90도 이상의 각도라면 그냥 가게 해줌
			Vec3 v = other->GetColliderCenter();
			v = v - player->GetWorldPos();
			v.Normalize();
			if (v.Dot(moveDir) <= 0.f)
				continue;

			collided = true;
			return;
		}
	}
	collided = false;
}

shared_ptr<ClientObject> DevApp1::FindObject(uint64 id)
{
	shared_ptr<GameObject> obj = _devScene->FindObject(id);
	if (obj == nullptr)
		return nullptr;

	return static_pointer_cast<ClientObject>(obj);
}


bool DevApp1::IsMyPlayer(uint64 id)
{
	return GetMyPlayerID() == id;
}

shared_ptr<MyPlayer> DevApp1::GetMyPlayer()
{
	return static_pointer_cast<MyPlayer>(_devScene->GetMyPlayer());
}

uint64 DevApp1::GetMyPlayerID()
{
	return GetMyPlayer()->GetID();
}

shared_ptr<Projectile> DevApp1::GetSphereBall(shared_ptr<GameObject> player)
{
	return _objectCreator->GetSphereBall(player);
}

void DevApp1::SetTargetMarkOn(const Vec3& pos)
{
	shared_ptr<GameObject> targetMarker = _objectCreator->GetTargetMarker();
	targetMarker->GetTransform()->SetWorldPosition(pos);
	Matrix matRotateY = GetLookCameraMatrix(targetMarker);
	
	targetMarker->GetTransform()->SetRotation(matRotateY);

	CUR_SCENE->AddTemproalObject(targetMarker);
}

void DevApp1::SetSphereBallOn(shared_ptr<GameObject> sphereBall)
{
	CUR_SCENE->AddTemproalObject(sphereBall);
}

void DevApp1::SetTargetMarkOff()
{
	CUR_SCENE->RemoveTemoporalObject(_objectCreator->GetTargetMarker());
}

void DevApp1::SetSphereBallOff(shared_ptr<GameObject> sphereBall)
{
	CUR_SCENE->RemoveTemoporalObject(sphereBall);
}

void DevApp1::AddTemporalObject(uint64 id, shared_ptr<GameObject> obj)
{
	_temporalObjects[id] = obj;
}

void DevApp1::DeleteTemporalObject(uint64 id)
{
	_temporalObjects.erase(id);
}

shared_ptr<GameObject> DevApp1::GetTemporalObject(uint64 id)
{
	shared_ptr<GameObject> obj = nullptr;
	obj = _temporalObjects[id];
	ASSERT_CRASH(obj != nullptr, "_temporalObjects[id] is nullptr");

	return obj;
}

Matrix DevApp1::GetLookCameraMatrix(shared_ptr<GameObject> obj)
{
	shared_ptr<Transform> cameraTr = CUR_SCENE->GetCamera()->GetTransform();
	shared_ptr<Transform> objTr = obj->GetTransform();
	Vec3 objPos = objTr->GetWorldPosition();

	Vec3 look = cameraTr->GetWorldPosition() - objPos;
	look *= -1;
	look.Normalize();
	Vec3 up = Vec3::Up;
	Vec3 right = up.Cross(look);
	right.Normalize();
	up = look.Cross(right);
	up.Normalize();
	
	Matrix mat = 
	{
		right.x, right.y, right.z, 0.0f,
		up.x, up.y, up.z, 0.0f,
		look.x, look.y, look.z, 0.0f,
		objPos.x, objPos.y, objPos.z, 1.0f
	};

	return mat;
}


shared_ptr<GameObject> DevApp1::CreateContainer(const ObjectInfo& info)
{
	shared_ptr<GameObject> obj = _objectCreator->CreateContainer(info);

	return obj;
}

shared_ptr<GameObject> DevApp1::CreateTower(const ObjectInfo& info)
{
	shared_ptr<GameObject> obj = _objectCreator->CreateTower(info);

	return obj;
}

shared_ptr<GameObject> DevApp1::CreateSmallMonster(const ObjectInfo& info)
{
	shared_ptr<GameObject> obj = _objectCreator->CreateSmallMonster(info);

	return obj;
}

shared_ptr<GameObject> DevApp1::CreateLargeMonster(const ObjectInfo& info)
{
	return shared_ptr<GameObject>();
}

shared_ptr<GameObject> DevApp1::CreateProjectile(const ObjectInfo& info)
{
	return shared_ptr<GameObject>();
}

void DevApp1::CreateManyCubes()
{

	for (int i = 0; i < 3000; ++i)
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		auto texture = RESOURCE->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		material->SetDiffuseMap(texture);
		MaterialLightDesc& desc = material->GetMaterialLightDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);
		RESOURCE->Add(L"Veigar", material);

		auto mesh = RESOURCE->GetBox();

		auto obj = make_shared<ClientObject>(ObjectType::OBJECT_TYPE_SMALL_MONSTER);

		static int32 tempID = 300;
		obj->GetInfo().set_objectid(tempID++);
		
		if (i < 20)
		{
			obj->GetTransform()->SetLocalPosition(Vec3((rand() % 50 - 25) + 200, 0.f, (rand() % 50 - 25) + 200));
			
		}
		else if (i < 1000)
		{
			obj->GetTransform()->SetLocalPosition(Vec3((rand() % 200 - 100) + 200, 0.f, (rand() % 200 - 100) + 200));
		}
		else
		{
			obj->GetTransform()->SetLocalPosition(Vec3((rand() % 300 - 300) + 100, 0.f, (rand() % 300 - 300) + 100));
		}
		
		obj->AddComponent(make_shared<MeshRenderer>(mesh, material));
		obj->GetMeshRenderer()->SetRenderPass(0);

		// Collider
		{
			auto collider = make_shared<SphereCollider>();
			collider->GetBoundingSphere().Center = obj->GetWorldPos();
			collider->GetBoundingSphere().Radius = 1.f;
			collider->SetGameObject(obj);
			obj->AddComponent(collider);
		}

		CUR_SCENE->Add(obj->GetID(), obj);
	}
}

