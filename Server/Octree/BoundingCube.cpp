#include "pch.h"
#include "BoundingCube.h"

BoundingCube::BoundingCube(const Vec3 mins, const Vec3 maxs)
	: _mins(mins), _maxs(maxs)
{
}


Vec3 BoundingCube::GetCenter()
{
	float cx = (_mins.x + _maxs.x) / 2.f;
	float cy = (_mins.y + _maxs.y) / 2.f;
	float cz = (_mins.z + _maxs.z) / 2.f;
	return Vec3(cx, cy, cz);
}

Vec3 BoundingCube::GetExtents()
{
	float dx = (_maxs.x - _mins.x) / 2.f;
	float dy = (_maxs.y - _mins.y) / 2.f;
	float dz = (_maxs.z - _mins.z) / 2.f;
	return Vec3(dx, dy, dz);
}

bool BoundingCube::Contains(const Vec3& point)
{
	if (!(point.x + EPSILON >= _mins.x && point.x <= _maxs.x + EPSILON))
		return false;
	if (!(point.y + EPSILON >= _mins.y && point.y <= _maxs.y + EPSILON))
		return false;
	if (!(point.z + EPSILON >= _mins.z && point.z <= _maxs.z + EPSILON))
		return false;

	return true;
}

bool BoundingCube::Contains(const BoundingCube& other)
{
	if (!(other._mins.x + EPSILON >= _mins.x && other._maxs.x <= _maxs.x + EPSILON))
		return false;
	if (!(other._mins.y + EPSILON >= _mins.y && other._maxs.y <= _maxs.y + EPSILON))
		return false;
	if (!(other._mins.z + EPSILON >= _mins.z && other._maxs.z <= _maxs.z + EPSILON))
		return false;

	return true;
}
