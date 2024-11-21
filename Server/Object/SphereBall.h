#pragma once
#include "Projectile.h"

class SphereBall : public Projectile
{
	using Super = Projectile;

public:
	SphereBall();
	virtual ~SphereBall();

	virtual void Update() override;

	

private:
	enum
	{
		SPHERE_BALL_SPEED = 15,
		MAX_DIST_SQUARED = 300,
	};

};

