#include "pch.h"
#include "MyPlayer.h"
#include "projectile.h"
#include "Transform.h"
#include "MyCamera.h"
#include "DevApp1.h"
#include "MeshRenderer.h"

MyPlayer::MyPlayer()
{
}

MyPlayer::~MyPlayer()
{
}

void MyPlayer::Awake()
{
	_sphereBall = G_DevApp->GetSphereBall(shared_from_this());
	_sphereBall->SetOwner(static_pointer_cast<MyPlayer>(shared_from_this()));
}

void MyPlayer::Update()
{
	Super::Update();

	uint64 now = ::GetTickCount64();

	if (_bTargetOn && (now - _targetOnStartTime) > TARGET_REMAIN_TIME)
	{
		G_DevApp->SetTargetMarkOff();
		_bTargetOn = false;
	}
	
}

void MyPlayer::UpdateIdle()
{
}

void MyPlayer::UpdateMove()
{
	if (HasCollided() == true)
		return;

	UpdateNextMovePos();
	uint64 now = ::GetTickCount64();
	
	if (--_firstPackets > 0 || now - _prevSendTime > SEND_TERM)
	{
		_prevSendTime = now;
		SendMovePacket();
	}
}

void MyPlayer::UpdateJump()
{
}

void MyPlayer::UpdateSkill()
{
	if (!_bSkillPacketSent)
	{
		_bSkillPacketSent = true;
		SendSkillPacket(/* Send Timestamp for SpecialSkill */false);
	}

	// 플레이어가 타켓을 향하게
	if (_bTargetOn && !_bRotateToTarget)
	{
		_bRotateToTarget = true;

		Vec3 look = GetTransform()->GetLook();
		Vec3 toTarget = _targetPos - GetWorldPos();
		look.Normalize(); 
		toTarget.Normalize();
		float cos = look.Dot(toTarget);

		if (cos < ROTATE_TO_TARGET_THRESHOLD)
		{
			float yaw = acos(cos);
			Vec3 crossProduct = look.Cross(toTarget);
			if (crossProduct.y < 0)
				yaw = -yaw;

			RotateYaw(yaw);
			SendChangeDirPacket(yaw);
		}
	}
	
	if (_bTargetOn && !_bShootBall)
	{
		_shootDelaySum += DT;

		if (_shootDelaySum > SHOOT_BALL_DELAY)
		{
			_sphereBall->SetPos(GetWorldPos(), _targetPos);

			SendCreateProjectilePacket(GetID(), ProjectileType::PROJECTILE_SPHERE_BALL, _sphereBall->GetStartPos(), _sphereBall->GetDir());

			_shootDelaySum = 0.f;

			_bShootBall = true;

			G_DevApp->SetSphereBallOn(_sphereBall);
		}
	}

	Super::UpdateSkill();

	return;
}

void MyPlayer::UpdateSpecialSkill()
{
	Super::UpdateSpecialSkill();

	if (!_bSkillPacketSent)
	{
		_bSkillPacketSent = true;
		SendSkillPacket(true, GetLastSpecialSkillUseTime());
	}
}

void MyPlayer::LateUpdate()
{
}

void MyPlayer::HandleCollided(const Protocol::S_Move& pkt)
{
	Vec3 targetPos = { pkt.movestat().posx(), pkt.movestat().posy(), pkt.movestat().posz() };
	Vec3 currPos = GetWorldPos();

	UpdateMoveInfo(pkt);
	GetTransform()->SetWorldPosition(targetPos);

	Vec3 dir = targetPos - currPos;
	dir.Normalize();
	float diff = (targetPos - currPos).Length();

	
}


void MyPlayer::UpdateNextMovePos()
{
	auto playerTransform = GetTransform();
	Vec3 playerPos = playerTransform->GetWorldPosition();

	float diff = DT * (_moveSpeed);
	
	_moveDir.Normalize();
	Vec3 dist = _moveDir * diff;
	playerPos += dist;

	bool collided = false;
	shared_ptr<ClientObject> thisObj = static_pointer_cast<ClientObject>(shared_from_this());
	G_DevApp->CheckCollision(thisObj, _moveDir, collided);
	
	if (collided)
	{
		SetCollide(true);
		return;
	}

	playerTransform->SetWorldPosition(playerPos);

	UpdateInfo();

}

void MyPlayer::SendLastMovePacket()
{
	SendMovePacket();
}



void MyPlayer::SetMoveDir(KEY_TYPE keyType)
{
	_firstPackets = 5;

	shared_ptr<Transform> cameraTr = _camera->GetTransform();
	shared_ptr<Transform> playerTr = GetTransform();

	Vec3 look = cameraTr->GetLook();
	look.y = 0.f;
	look.Normalize();
	Vec3 right = cameraTr->GetRight();
	right.y = 0.f;
	right.Normalize();

	_yaw = atan2(look.x, look.z);

	switch (keyType)
	{
	case KEY_TYPE::W:
		_moveDir = look;
		_yaw += 3.14f;
		break;
	case KEY_TYPE::A:
		_moveDir = -right;
		_yaw += 3.14f / 2.f;
		break;
	case KEY_TYPE::S:
		_moveDir = -look;
		break;
	case KEY_TYPE::D:
		_moveDir = right;
		_yaw += 4.71f;
		break;
	default:
		assert(false);
		break;
	}

	RotateYaw(_yaw);

	SendChangeDirPacket(_yaw);
}

void MyPlayer::SpecialAttack()
{
	uint64 now = ::GetTickCount64();
	if (now - GetLastSpecialSkillUseTime() < SPECIAL_SKILL_COOL_TIME)
		return;

	SetLastSpecialSkillUseTime(now);
	SetSendSkillPacketReady();
	SetState(SPECIAL_SKILL, true);
	SetSkillReady();
}

void MyPlayer::Pick(POINT point)
{
	shared_ptr<GameObject> picked = CUR_SCENE->Pick(point.x, point.y);
	if (picked)
	{
		shared_ptr<ClientObject> obj = static_pointer_cast<ClientObject>(picked);
		ObjectType type = obj->GetObjectType();

		if (type == ObjectType::OBJECT_TYPE_SMALL_MONSTER ||
			type == ObjectType::OBJECT_TYPE_LARGE_MONSTER)
		{
			Vec3 pickedPos = picked->GetTransform()->GetWorldPosition();
			_targetPos = pickedPos;

			Vec3 playerPos = GetWorldPos();
			Vec3 dir = playerPos - pickedPos;
			dir.Normalize();

			pickedPos += dir * 1.f;
			G_DevApp->SetTargetMarkOn(pickedPos);

			_targetOnStartTime = ::GetTickCount64();
			_bTargetOn = true;
			_shootDelaySum = 0.f;
			_bRotateToTarget = false;
		}
	}
}

void MyPlayer::DisappearShootBall()
{
	if (!_bShootBall)
		return;

	G_DevApp->SetSphereBallOff(_sphereBall);
	_bShootBall = false;
	_shootDelaySum = 0.f;
}


void MyPlayer::UpdateCameraRotate(int32 xDiff)
{
	float value = xDiff / 800.f;
	_camera->RotateCameraY(value);
}

void MyPlayer::SetSendSkillPacketReady()
{
	_bSkillPacketSent = false; 
	Super::SetSkillReady();
}




