#include "pch.h"
#include "Projectile.h"
#include "Player.h"
#include "GameRoom.h"


void Projectile::Update()
{
	Super::Update();

	Vec3 currPos = GetWorldPos();
	if ((currPos - _startPos).LengthSquared() > MAX_DIST_SQUARED)
	{
		cout << "MAX_DIST" << endl;
		GetGameRoom()->AddTrash(GetID());
		return;
	}
	
	Vec3 offset = _dir * SPHERE_BALL_MOVE_DIST_PER_TICK;
	Vec3 nextPos = currPos + offset;

	SetPos(nextPos);

	ProjectileRef obj = static_pointer_cast<Projectile>(shared_from_this());
	bool collided = false;
	GameObjectRef target = nullptr;
	GetGameRoom()->CheckCollision(obj, collided, target);

	if (collided)
	{
		ASSERT_CRASH(target != nullptr, "Collided Target is nullptr");
		GetGameRoom()->AddTrash(target->GetID());
		GetGameRoom()->AddTrash(GetID());
	}
}


void Projectile::SetProjectileInfo(PlayerRef player, Vec3 startPos, Vec3 dir)
{
	_owner = player;
	_dir = dir;
	_startPos = startPos;

	_info.set_state(MOVE);
	_info.set_level(player->GetLevel());
	Protocol::MoveStat* movestat = _info.mutable_movestat();
	movestat->set_posx(startPos.x);
	movestat->set_posy(startPos.y);
	movestat->set_posz(startPos.z);
	movestat->set_lookx(dir.x);
	movestat->set_looky(dir.y);
	movestat->set_lookz(dir.z);

	SetInfo(_info);
}