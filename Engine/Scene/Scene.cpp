#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "Camera.h"
#include "Terrain.h"
#include "Transform.h"


Scene::Scene()
{
	// 전체 월드를 반 잘라서 옥트리 시작 루트를 잡아줌

	const Vec3 mins = { -ROOT_CUBE_X, -ROOT_CUBE_Y, -ROOT_CUBE_Z };
	const Vec3 maxs = { ROOT_CUBE_X, ROOT_CUBE_Y, ROOT_CUBE_Z };
	const Vec3 startCenter = { 0, 0, 0 };
	const Vec3 startExtents = { ROOT_CUBE_X, ROOT_CUBE_Y, ROOT_CUBE_Z };
	BoundingCube cube(startCenter, startExtents);

	_octreeRoot = make_shared<Octree>(cube);

	
}

void Scene::Start()
{
	unordered_map<uint64, shared_ptr<GameObject>> objects = _objects;

	for (auto& pair : objects)
	{
		pair.second->Start();
	}
}

void Scene::Update()
{
	{
		unordered_map<uint64, shared_ptr<GameObject>> objects = _objects;

		for (auto& pair : objects)
		{
			pair.second->Update();
		}
	}

	// TemporalObject

	list<shared_ptr<GameObject>> temporalObject = _temporalObjects;

	for (const auto& obj : temporalObject)
	{
		obj->Update();
	}
	

	// INSTANCING

	vector<shared_ptr<GameObject>> temp;
	temp.reserve(_objects.size());
	vector<shared_ptr<GameObject>> objects;
	objects.reserve(_objects.size());

	if (_myCamera == nullptr)
		return;

	BoundingFrustum frustum = _myCamera->GetCamera()->GetBoundingFrustum();
	// 옥트리로 1차적으로 걸러내기
	{
		if (_player != nullptr)
		{
			_octreeRoot->GetGameObjects(temp, frustum, 0);

			bool check = false;
			for (auto ptr : temp)
			{
				if (ptr->GetModelAnimator() != nullptr)
					check = true;
			}

			if (!check)
			{
				int a = 3;
			}
		}
	}

	for (shared_ptr<GameObject>& object : temp)
	{
		shared_ptr<BaseCollider> collider = object->GetCollider();
		if (collider->Intersects(frustum))
		{
			objects.push_back(object);
		}
	}


	for (auto obj : temporalObject)
	{
		objects.push_back(obj);
	}

	_totalObjectCount = _objects.size();
	_firstCulledCount = temp.size();
	_culledCount = objects.size();

	INSTANCING->Render(objects);

	// TODO : Blend Object 그리기

	/*if (!_temporalBlendObjects.empty())
	{
		list<shared_ptr<GameObject>> v = _temporalBlendObjects;
		for (shared_ptr<GameObject> obj : v)
		{
			auto renderer = obj->GetFixedComponent(ComponentType::MeshRenderer);
			
		}
	}*/
}

void Scene::LateUpdate()
{
	unordered_map<uint64, shared_ptr<GameObject>> objects = _objects;

	for (auto& pair : objects)
	{
		pair.second->LateUpdate();
	}

	//CheckCollision();
}

void Scene::Add(uint64 id, shared_ptr<GameObject> object)
{
	_objects[id] = object;

	if (object->GetCamera() != nullptr)
	{
		_cameras.insert(object);
		return;
	}

	if (object->GetLight() != nullptr)
	{
		_lights.insert(object);
		return;
	}

	_octreeRoot->Insert(object);
}

void Scene::Remove(uint64 id)
{
	shared_ptr<GameObject> object;

	auto findIt = _objects.find(id);

	if (findIt != _objects.end())
	{
		object = findIt->second;
		_objects.erase(id);
	}

	_cameras.erase(object);

	_lights.erase(object);

	_octreeRoot->Remove(object);
}

shared_ptr<GameObject> Scene::FindObject(uint64 id)
{
	shared_ptr<GameObject> obj = nullptr;
	
	if (_objects.find(id) != _objects.end())
	{
		obj = _objects[id];
	}

	return obj;
}


uint32 Scene::GetFarPlaneInRootCube()
{
	Vec3 pos = _myCamera->GetCamera()->GetTransform()->GetWorldPosition();
	Vec3 look = _myCamera->GetCamera()->GetTransform()->GetLook();
	look.y = 0;
	look.Normalize();
	
	float maxX = (look.x > 0) ? (ROOT_CUBE_MAX - pos.x) / look.x : (look.x < 0) ? (-ROOT_CUBE_MAX - pos.x) / look.x : FLT_MAX;
	float maxY = (look.y > 0) ? (ROOT_CUBE_Y - pos.y) / look.y : (look.y < 0) ? (-ROOT_CUBE_Y - pos.y) / look.y : FLT_MAX;
	float maxZ = (look.z > 0) ? (ROOT_CUBE_MAX - pos.z) / look.z : (look.z < 0) ? (-ROOT_CUBE_MAX - pos.z) / look.z : FLT_MAX;
	
	// 가장 작은 max 값을 선택하여 최대 거리를 결정 
	float maxV = std::min({ maxX, maxY, maxZ }); 
	// 최대 거리를 반환 

	/*float ret = maxX == maxV ? pos.x : maxY == maxV ? pos.y : pos.z;
	ret += maxV;*/
	
	return static_cast<uint32>(maxV);
}


void Scene::AddTemproalObject(shared_ptr<GameObject> object)
{
	_temporalObjects.push_back(object);
}

void Scene::AddBlendTemproalObject(shared_ptr<GameObject> object)
{
	_temporalBlendObjects.push_back(object);
}

void Scene::RemoveTemoporalObject(shared_ptr<GameObject> object)
{
	auto findIt = std::find(_temporalObjects.begin(), _temporalObjects.end(), object);
	if (findIt != _temporalObjects.end())
		_temporalObjects.erase(findIt);
}


void Scene::RemoveBlendTemoporalObject(shared_ptr<GameObject> object)
{
	auto findIt = std::find(_temporalBlendObjects.begin(), _temporalBlendObjects.end(), object);
	if (findIt != _temporalBlendObjects.end())
		_temporalBlendObjects.erase(findIt);
}

shared_ptr<class GameObject> Scene::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GetCamera()->GetCamera();

	float width = GRAPHICS->GetViewport().GetWidth();
	float height = GRAPHICS->GetViewport().GetHeight();

	Matrix matProjection = camera->GetProjectionMatrix();
	Matrix matView = camera->GetViewMatrix();
	Matrix invMatView = matView.Invert();

	// screenX -> View
	float viewX = (+2.f * screenX / width - 1.f) / matProjection(0, 0);
	float viewY = (-2.f * screenY / height + 1.f) / matProjection(1, 1);

	const auto& gameObjects = GetObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& pair : gameObjects)
	{
		shared_ptr<GameObject> gameObject = pair.second;

		if (gameObject->GetCollider() == nullptr)
			continue;

		// ViewSpace에서 Ray 정의
		Vec4 rayOrigin = Vec4(0.f, 0.f, 0.f, 1.f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.f, 0.f);

		// World로
		Vec3 worldRayOrigin = XMVector3TransformCoord(rayOrigin, invMatView);
		Vec3 worldRayDir = XMVector3TransformNormal(rayDir, invMatView);
		worldRayDir.Normalize();
		Ray ray = Ray(worldRayOrigin, worldRayDir);

		float distance = 0.f;
		if (gameObject->GetCollider()->Intersects(ray, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	// Terrain Picking
#if 0
	for (auto& pair : gameObjects)
	{
		shared_ptr<GameObject> gameObject = pair.second;

		if (gameObject->GetTerrain() == nullptr)
			continue;

		Vec3 pickPos;
		float distance = 0.f;
		if (gameObject->GetTerrain()->Pick(screenX, screenY, OUT pickPos, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}
#endif

	return picked;
}

void Scene::CheckCollision()
{
	vector<shared_ptr<BaseCollider>> colliders;
	_collided = false;

	for (auto& pair : _objects)
	{
		shared_ptr<GameObject> object = pair.second;

		if (object->GetCollider() == nullptr)
			continue;
		if (object->shared_from_this() == _player)
			continue;
		colliders.push_back(object->GetCollider());
	}
	
	// BruteForce
	for (auto& other : colliders)
	{
		if (_player->GetCollider()->Intersects(other))
		{
			_collided = true;
			return;
		}
	}
	_collided = false;
}


