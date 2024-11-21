#pragma once
#include "GameObject.h"

class Player;

class Projectile : public GameObject
{
	using Super = GameObject;

public:
	Projectile(ProjectileType projectileType) : _projectileType(projectileType) {}
	virtual ~Projectile() {}

	virtual void Update() override;
	
	ProjectileType GetProjectileType() { return _projectileType; }
	PlayerRef GetOwner() { return _owner.lock(); }
	Vec3 GetDir() { return _dir; }
	void SetProjectileInfo(PlayerRef player, Vec3 startPos, Vec3 dir);


private:
	enum
	{
		MAX_DIST_SQUARED = 1000,
	};
	const float SPHERE_BALL_MOVE_DIST_PER_TICK = 0.35f;

protected:
	ProjectileType _projectileType;
	weak_ptr<Player> _owner;
	Vec3 _startPos;
	Vec3 _dir;
};

