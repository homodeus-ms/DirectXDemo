#pragma once
#include "ClientObject.h"
class Projectile : public ClientObject
{
	using Super = ClientObject;

public:
	Projectile();
	virtual ~Projectile();

	virtual void Update() override;

	void SetOwner(shared_ptr<class MyPlayer> player);
	void SetPos(Vec3 pos, Vec3 targetPos);
	void SetProjectileInfo(Vec3 startPos, Vec3 dir);
	Vec3 GetStartPos() { return _startPos; }
	Vec3 GetDir() { return _dir; }
	void SetID(uint64 id) { _info.set_objectid(id); }
	
	shared_ptr<class MyPlayer> GetOwner();

private:
	enum 
	{ 
		SPHERE_BALL_SPEED = 22,
		MAX_DIST_SQUARED = 1000,
	};
	const Vec3 START_OFFSET = { -1.f, 1.5f, -0.5f };

	weak_ptr<MyPlayer> _owner;
	Vec3 _startPos;
	Vec3 _dir;
};

