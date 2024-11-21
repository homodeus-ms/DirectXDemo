#include "Projectile.h"
#include "Projectile.h"
#include "pch.h"
#include "Projectile.h"
#include "MyPlayer.h"
#include "DevApp1.h"
#include "Transform.h"

Projectile::Projectile()
	: Super(ObjectType::OBJECT_TYPE_PROJECTILE)
{
}

Projectile::~Projectile()
{
}

void Projectile::Update()
{
	Vec3 currPos = GetTransform()->GetWorldPosition();
	if ((currPos - _startPos).LengthSquared() > MAX_DIST_SQUARED)
	{
		shared_ptr<Player> owner = GetOwner();
		if (owner)
			GetOwner()->DisappearShootBall();
		else
			G_DevApp->SetSphereBallOff(shared_from_this());
		return;
	}
	float deltaTime = DT;
	float diff = DT * SPHERE_BALL_SPEED;
	Vec3 offset = _dir * diff;
	Vec3 nextPos = currPos + offset;
	GetTransform()->SetWorldPosition(nextPos);
}



void Projectile::SetOwner(shared_ptr<class MyPlayer> player)
{
	_owner = player;
	
	Matrix matWorld = player->GetTransform()->GetWorldMatrix();
	Vec3 offset = START_OFFSET;
	offset = Vec3::Transform(offset, matWorld);
	_startPos = player->GetWorldPos() + offset;
	GetTransform()->SetWorldPosition(_startPos);
}

void Projectile::SetPos(Vec3 pos, Vec3 targetPos)
{
	Vec3 playerWorldRotation = GetOwner()->GetTransform()->GetWorldRotation();

	Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(
		playerWorldRotation.y,   // yaw (Y축 회전)
		playerWorldRotation.x,   // pitch (X축 회전)
		playerWorldRotation.z    // roll (Z축 회전)
	);

	Vec3 v = START_OFFSET;
	v = Vec3::TransformNormal(v, rotationMatrix);

	_startPos = pos + v;
	GetTransform()->SetWorldPosition(_startPos);

	_dir = targetPos - _startPos;
	_dir.Normalize();

}

void Projectile::SetProjectileInfo(Vec3 startPos, Vec3 dir)
{
	_startPos = startPos;
	_dir = dir;
}

shared_ptr<class MyPlayer> Projectile::GetOwner()
{
	return _owner.lock();
}
