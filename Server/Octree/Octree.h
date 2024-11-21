#pragma once

#include "BoundingCube.h"
#include "GameObject.h"

enum
{
	MIN_CUBE_DIMENSION = 5,
};

class Octree
{
public:
	Octree(BoundingCube cube);

	vector<shared_ptr<GameObject>>& GetGameObjects(const BoundingCube& cube);
	boolean Insert(const shared_ptr<GameObject>& object);
	void Remove(const shared_ptr<GameObject> object);

private:
	void CreateChildren();

private:
	BoundingCube _boundingCube;

	shared_ptr<Octree> _tLeftFront;
	shared_ptr<Octree> _tLeftBack;
	shared_ptr<Octree> _tRightFront;
	shared_ptr<Octree> _tRightBack;
	shared_ptr<Octree> _bLeftFront;
	shared_ptr<Octree> _bLeftBack;
	shared_ptr<Octree> _bRightFront;
	shared_ptr<Octree> _bRightBack;

	vector<shared_ptr<GameObject>> _gameObjects;
};

