#include "pch.h"
#include "SphereBall.h"

SphereBall::SphereBall()
	: Super(ProjectileType::PROJECTILE_SPHERE_BALL)
{
	_info.mutable_movestat()->set_speed(SPHERE_BALL_SPEED);
}

SphereBall::~SphereBall()
{
}

void SphereBall::Update()
{
	Super::Update();
}


