#pragma once

#include "Octree.h"

enum
{
	// Octree Start Cube Size
	ROOT_CUBE_X = 301,
	ROOT_CUBE_Y = 50,
	ROOT_CUBE_Z = 301,
	ROOT_CUBE_MAX = 301,
};


class Scene
{
public:
	Scene();

	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();

	virtual void Add(uint64 id, shared_ptr<GameObject> object);
	virtual void Remove(uint64 id);


	// Getter
	unordered_map<uint64, shared_ptr<GameObject>> GetObjects() { return _objects; }
	shared_ptr<GameObject> FindObject(uint64 id);
	shared_ptr<GameObject> GetMyPlayer() { return _player; }
	shared_ptr<Octree> GetOctreeRoot() { return _octreeRoot; }
	vector<shared_ptr<GameObject>>& GetBoundingObjects(BoundingCube cube) { return _octreeRoot->GetGameObjects(cube); }
	uint32 GetFarPlaneInRootCube();


	// 카메라와 라이트 하나씩만 있다 가정
	shared_ptr<GameObject> GetCamera() { return _cameras.empty() ? nullptr : *_cameras.begin(); }
	shared_ptr<GameObject> GetLight() { return _lights.empty() ? nullptr : *_lights.begin(); }
	bool HasCollided() { return _collided; }

	// Setter
	void SetMyPlayer(shared_ptr<GameObject>& player) { _player = player; }
	void SetMyCamera(shared_ptr<GameObject> myCamera) { _myCamera = myCamera; }
	void InsertToOctree(shared_ptr<GameObject> gameObject) { _octreeRoot->Insert(gameObject); }
	void SetCollidedFlagOff() { _collided = false; }

	void AddTemproalObject(shared_ptr<GameObject> object);
	void AddBlendTemproalObject(shared_ptr<GameObject> object);
	void RemoveTemoporalObject(shared_ptr<GameObject> object);
	void RemoveBlendTemoporalObject(shared_ptr<GameObject> object);

	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);
	void CheckCollision();

private:
	
	

public:
	// TEST
	int32 _frustumSize = -1;
	int32 _totalObjectCount = -1;
	int32 _firstCulledCount = -1;
	int32 _culledCount = -1;

private:
	//unordered_set<shared_ptr<GameObject>> _objects;
	unordered_map<uint64, shared_ptr<GameObject>> _objects;
	list<shared_ptr<GameObject>> _temporalObjects;
	list<shared_ptr<GameObject>> _temporalBlendObjects;
	
	// Cache Camera
	unordered_set<shared_ptr<GameObject>> _cameras;
	// Cache Light
	unordered_set<shared_ptr<GameObject>> _lights;

	// Cache MyCamera??
	shared_ptr<GameObject> _myCamera;
	shared_ptr<GameObject> _player;

	shared_ptr<Octree> _octreeRoot;

	bool _collided = false;
};

