#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "BaseCollider.h"
#include "Camera.h"
#include "Terrain.h"
#include "Transform.h"


Scene::Scene()
{
	// ��ü ���带 �� �߶� ��Ʈ�� ���� ��Ʈ�� �����

	const Vec3 mins = { -ROOT_CUBE_X, -ROOT_CUBE_Y, -ROOT_CUBE_Z };
	const Vec3 maxs = { ROOT_CUBE_X, ROOT_CUBE_Y, ROOT_CUBE_Z };

	BoundingCube cube(mins, maxs);

	_objectsRoot = make_shared<Octree>(cube);
}

void Scene::Start()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->Start();
	}
}

void Scene::Update()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->Update();
	}

	// INSTANCING
	vector<shared_ptr<GameObject>> temp;

	
	// ��Ʈ���� ��ü ������ �ɷ�����
	{
		if (_player != nullptr)
		{
			const int32 BOX_SIZE = 100;

			auto transform = _player->GetTransform();
			Vec3 look = transform->GetLook();
			Vec3 worldPos = transform->GetWorldPosition();
			Vec3 mins = { worldPos.x - BOX_SIZE, worldPos.y, worldPos.z - BOX_SIZE };
			Vec3 maxs = { worldPos.x + BOX_SIZE, worldPos.y, worldPos.z + BOX_SIZE };
			BoundingCube boundingCube(mins, maxs);

			vector<shared_ptr<GameObject>>& v = _objectsRoot->GetGameObjects(boundingCube);

			temp.insert(temp.end(), v.begin(), v.end());

			_totalObjectCount = _objects.size();
			_culledCount = temp.size();
		}
	}


	INSTANCING->Render(temp);
}

void Scene::LateUpdate()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->LateUpdate();
	}

	CheckCollision();
}

void Scene::Add(shared_ptr<GameObject> object)
{
	_objects.insert(object);

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

	_objectsRoot->Insert(object);
}

void Scene::Remove(shared_ptr<GameObject> object)
{
	_objects.erase(object);

	_cameras.erase(object);

	_lights.erase(object);

	_objectsRoot->Remove(object);
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

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetCollider() == nullptr)
			continue;

		// ViewSpace���� Ray ����
		Vec4 rayOrigin = Vec4(0.f, 0.f, 0.f, 1.f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.f, 0.f);

		// World��
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

	for (auto& gameObject : gameObjects)
	{
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

	return picked;
}

void Scene::CheckCollision()
{
	vector<shared_ptr<BaseCollider>> colliders;
	_collided = false;

	for (const shared_ptr<GameObject>& object : _objects)
	{
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
