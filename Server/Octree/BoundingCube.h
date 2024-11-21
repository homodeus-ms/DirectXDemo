#pragma once


class BoundingCube
{

public:
	BoundingCube() {};
	BoundingCube(const Vec3 mins, const Vec3 maxs);
	
	
	uint32 GetWidth() { return static_cast<uint32>(_maxs.x - _mins.x); }
	uint32 GetHeight() { return static_cast<uint32>(_maxs.y - _mins.y); }
	uint32 GetDepth() { return static_cast<uint32>((_maxs.z - _mins.z) / 2); }

	Vec3 GetMins() { return _mins; }
	Vec3 GetMaxs() { return _maxs; }

	Vec3 GetCenter();
	Vec3 GetExtents();

	bool Contains(const Vec3& point);
	bool Contains(const BoundingCube& other);
	

public:
	
	const float EPSILON = 0.0001f;

	Vec3 _mins;
	Vec3 _maxs;


};

