#include "pch.h"
#include "Octree.h"
#include "Transform.h"

Octree::Octree(BoundingCube cube)
    : _boundingCube(cube)
{
    CreateChildren();
}

vector<shared_ptr<GameObject>>& Octree::GetGameObjects(const BoundingCube& cube)
{
    if (!_boundingCube.Contains(cube) || _tLeftFront == nullptr)
        return _gameObjects;

    if (_tLeftFront->_boundingCube.Contains(cube))
    {
        return _tLeftFront->GetGameObjects(cube);
    }

    if (_tLeftBack->_boundingCube.Contains(cube))
    {
        return _tLeftBack->GetGameObjects(cube);
    }

    if (_tRightFront->_boundingCube.Contains(cube))
    {
        return _tRightFront->GetGameObjects(cube);
    }

    if (_tRightBack->_boundingCube.Contains(cube))
    {
        return _tRightBack->GetGameObjects(cube);
    }

    if (_bLeftFront->_boundingCube.Contains(cube))
    {
        return _bLeftFront->GetGameObjects(cube);
    }

    if (_bLeftBack->_boundingCube.Contains(cube))
    {
        return _bLeftBack->GetGameObjects(cube);
    }

    if (_bRightFront->_boundingCube.Contains(cube))
    {
        return _bRightFront->GetGameObjects(cube);
    }

    if (_bRightBack->_boundingCube.Contains(cube))
    {
        return _bRightBack->GetGameObjects(cube);
    }

    return _gameObjects;
}

void Octree::GetGameObjects(OUT vector<shared_ptr<GameObject>>& objects, BoundingFrustum frustum, int32 depth)
{
    if (!(_boundingCube.Intersects(frustum)) || _tLeftFront == nullptr || IsGameObjectsEmpty())
        return;

    if (depth == FRUSTUM_OBJECT_FIND_DEPTH)
    {
        objects.insert(objects.end(), _gameObjects.begin(), _gameObjects.end());
        return;
    }
    if (_tLeftFront->_boundingCube.Intersects(frustum))
    {
        _tLeftFront->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_tLeftBack->_boundingCube.Intersects(frustum))
    {
        _tLeftBack->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_tRightFront->_boundingCube.Intersects(frustum))
    {
        _tRightFront->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_tRightBack->_boundingCube.Intersects(frustum))
    {
        _tRightBack->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_bLeftFront->_boundingCube.Intersects(frustum))
    {
        _bLeftFront->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_bLeftBack->_boundingCube.Intersects(frustum))
    {
        _bLeftBack->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_bRightFront->_boundingCube.Intersects(frustum))
    {
        _bRightFront->GetGameObjects(objects, frustum, depth + 1);
    }

    if (_bRightBack->_boundingCube.Intersects(frustum))
    {
        _bRightBack->GetGameObjects(objects, frustum, depth + 1);
    }

}


boolean Octree::Insert(const shared_ptr<GameObject>& object)
{
    Vec3 pos = object->GetTransform()->GetWorldPosition();

    if (!_boundingCube.Contains(pos))
        return false;

    _gameObjects.push_back(object);

    if (_tLeftFront != nullptr)
    {
        _tLeftFront->Insert(object);
        _tLeftBack->Insert(object);
        _tRightFront->Insert(object);
        _tRightBack->Insert(object);

        _bLeftFront->Insert(object);
        _bLeftBack->Insert(object);
        _bRightFront->Insert(object);
        _bRightBack->Insert(object);
    }

    return true;
}

void Octree::Remove(const shared_ptr<GameObject> object)
{
    Vec3 pos = object->GetTransform()->GetWorldPosition();

    if (!_boundingCube.Contains(pos))
        return;

    auto findIt = std::find(_gameObjects.begin(), _gameObjects.end(), object);
    if (findIt != _gameObjects.end())
        _gameObjects.erase(findIt);

    if (_tLeftFront != nullptr)
    {
        _tLeftFront->Remove(object);
        _tLeftBack->Remove(object);
        _tRightFront->Remove(object);
        _tRightBack->Remove(object);

        _bLeftFront->Remove(object);
        _bLeftBack->Remove(object);
        _bRightFront->Remove(object);
        _bRightBack->Remove(object);
    }
}

void Octree::CreateChildren()
{
    if (_boundingCube.GetWidth() < MIN_CUBE_DIMENSION ||
        _boundingCube.GetHeight() < MIN_CUBE_DIMENSION ||
        _boundingCube.GetDepth() < MIN_CUBE_DIMENSION)
    {
        return;
    }

    float minX = _boundingCube._mins.x;
    float maxX = _boundingCube._maxs.x;
    float minY = _boundingCube._mins.y;
    float maxY = _boundingCube._maxs.y;
    float minZ = _boundingCube._mins.z;
    float maxZ = _boundingCube._maxs.z;
    
    Vec3 center = { (minX + maxX) / 2.f, (minY + maxY) / 2.f, (minZ + maxZ) / 2.f };

    // TopLeftFront, TopLeftBack
    {
        Vec3 mins(minX, center.y, center.z);
        Vec3 maxs(center.x, maxY, maxZ);
        BoundingCube cube(mins, maxs);
        _tLeftFront = make_shared<Octree>(cube);

        mins = { minX, center.y, minZ };
        maxs = { center.x, maxY, center.z };
        BoundingCube cube2(mins, maxs);
        _tLeftBack = make_shared<Octree>(cube2);
    }
    // TopRightFront, TopRightBack
    {
        Vec3 mins(center.x, center.y, center.z);
        Vec3 maxs(maxX, maxY, maxZ);
        BoundingCube cube(mins, maxs);
        _tRightFront = make_shared<Octree>(cube);

        mins = { center.x, center.y, minZ };
        maxs = { maxX, maxY, center.z };
        BoundingCube cube2(mins, maxs);
        _tRightBack = make_shared<Octree>(cube2);
    }
    // BottomLeftFront, BottomLeftBack
    {
        Vec3 mins(minX, minY, center.z);
        Vec3 maxs(center.x, center.y, maxZ);
        BoundingCube cube(mins, maxs);
        _bLeftFront = make_shared<Octree>(cube);

        mins = { minX, minY, minZ };
        maxs = { center.x, center.y, center.z };
        BoundingCube cube2(mins, maxs);
        _bLeftBack = make_shared<Octree>(cube2);
    }
    // BottomRightFront, BottomRightBack
    {
        Vec3 mins(center.x, minY, center.z);
        Vec3 maxs(maxX, center.y, maxZ);
        BoundingCube cube(mins, maxs);
        _bRightFront = make_shared<Octree>(cube);

        mins = { center.x, minY, minZ };
        maxs = { maxX, center.y, center.z };
        BoundingCube cube2(mins, maxs);
        _bRightBack = make_shared<Octree>(cube2);
    }
   
}
