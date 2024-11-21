#pragma once


class BoundingCube
{

public:
	BoundingCube() {};
	BoundingCube(const Vec3 mins, const Vec3 maxs);
	
	
	uint32 GetWidth() { return _maxs.x - _mins.x; }
	uint32 GetHeight() { return _maxs.y - _mins.y; }
	uint32 GetDepth() { return (_maxs.z - _mins.z) / 2; }

	Vec3 GetMins() { return _mins; }
	Vec3 GetMaxs() { return _maxs; }

	Vec3 GetCenter();
	Vec3 GetExtents();
	const BoundingBox& GetBoundingBox() { return _boundingBox; }

	bool Contains(const Vec3& point);
	bool Contains(const BoundingCube& other);
	bool Intersects(DirectX::BoundingFrustum cameraFrustum);

public:
	// Near Top Left(0)
	// Near Top Right(1)
	// Near Bottom Right(2)
	// Near Bottom Left(3)
	// Far Top Left(4)
	// Far Top Right(5)
	// Far Bottom Right(6)
	// Far Bottom Left(7)
	enum
	{
		BOTTOM_PLANE = 2,
		FAR_PLANE = 4,
	};
	
	const float EPSILON = 0.0001f;

	Vec3 _mins;
	Vec3 _maxs;

	BoundingBox _boundingBox;

};

