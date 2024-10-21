#pragma once

#include "Octree.h"

enum
{
	ROOT_CUBE_X = 1000,
	ROOT_CUBE_Y = 1000,
	ROOT_CUBE_Z = 1000,
};

class Scene
{
public:
	Scene();

	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();

	virtual void Add(shared_ptr<GameObject> object);
	virtual void Remove(shared_ptr<GameObject> object);

	unordered_set<shared_ptr<GameObject>> GetObjects() { return _objects; }
	
	// 카메라와 라이트 하나씩만 있다 가정
	shared_ptr<GameObject> GetCamera() { return _cameras.empty() ? nullptr : *_cameras.begin(); }
	shared_ptr<GameObject> GetLight() { return _lights.empty() ? nullptr : *_lights.begin(); }

	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);
	void SetPlayer(shared_ptr<GameObject>& player) { _player = player; }

	void CheckCollision();
	bool HasCollided() { return _collided; }
	void SetCollidedFlagOff() { _collided = false; }

public:
	// TEST
	int32 _frustumSize = -1;
	int32 _totalObjectCount = -1;
	int32 _culledCount = -1;

private:
	unordered_set<shared_ptr<GameObject>> _objects;
	// Cache Camera
	unordered_set<shared_ptr<GameObject>> _cameras;
	// Cache Light
	unordered_set<shared_ptr<GameObject>> _lights;

	shared_ptr<GameObject> _player;

	shared_ptr<Octree> _objectsRoot;

	bool _collided = false;
};

