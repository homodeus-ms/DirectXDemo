#include "pch.h"
#include "DevApp1.h"
#include "Player.h"
#include "Transform.h"
#include "ModelAnimator.h"
#include "MyCamera.h"


Player::Player()
	: Super(ObjectType::OBJECT_TYPE_PLAYER)
{
}

void Player::Update()
{
	Super::Update();

	if (GetState() == SKILL)
	{
		uint64 now = ::GetTickCount64();

		if (now - _skillStart > SKILL_END_TICK)
		{
			_skillEnd = true;
			SetState(IDLE, false);
		}
		return;
	}

	if (GetState() == SPECIAL_SKILL)
	{
		uint64 now = ::GetTickCount64();
		if (now - _skillStart > SPECIAL_SKILL_END_TICK)
		{
			_skillEnd = true;
			SetState(IDLE, false);
		}
		return;
	}

	// TEMP Jump 처리
	if (_jumping)
	{
		DoJump();
		return;
	}

}

void Player::UpdateIdle()
{
}

void Player::UpdateMove()
{
	shared_ptr<Transform> tr = GetTransform();
	Vec3 targetPos = GetWorldPos();
	Vec3 currPos = tr->GetWorldPosition();
	Vec3 dir = targetPos - currPos;
	dir.Normalize();

	float diff = (targetPos - currPos).LengthSquared();

	if (diff < 0.01f)
	{
		tr->SetWorldPosition(currPos);
		return;
	}

	float offset = DT * _moveSpeed;
	currPos += dir * offset;
	tr->SetWorldPosition(currPos);
	tr->SetLocalRotation({ _info.movestat().rotatex(), _info.movestat().rotatey(), _info.movestat().rotatez() });
	
}

void Player::UpdateJump()
{
}

void Player::UpdateSkill()
{
	uint64 now = ::GetTickCount64();
	if (now - _skillStart > SKILL_END_TICK)
	{
		_skillEnd = true;
		SetState(IDLE, true);
	}

}

void Player::UpdateSpecialSkill()
{
	uint64 now = ::GetTickCount64();
	if (now - _skillStart > SPECIAL_SKILL_END_TICK)
	{
		_skillEnd = true;
		SetState(IDLE, true);
	}
}

void Player::RotateY(uint64 id, bool toLeft, bool isMyMove)
{
	
}

void Player::Jump()
{
	// TODO : Jump 기능 정리 중
	//_camera->GoLerp(true);
	_jumpStartTime = ::GetTickCount64();
	SetState(JUMP, false);
	_jumping = true;

	/*_camera->GoLerp(true);
	SetState(JUMP, true);
	_jumpStartTime = ::GetTickCount64();*/
}

void Player::Attack()
{
	_skillStart = ::GetTickCount64();
	SetState(SKILL, false);
	
	_skillEnd = false;
}



// 점프 기능 정리 중

void Player::DoJump()
{
	uint64 now = ::GetTickCount64();

	shared_ptr<Transform> playerTransform = GetTransform();

	float diff;
	Vec3 playerLook;

	if (now - _jumpStartTime < _jumpTime)
	{
		diff = DT * _moveSpeed * JUMPING_UP_SPEED_DIFF;
		playerLook = -playerTransform->GetUp();
	}
	else if (GetTransform()->GetWorldPosition().y > 0.2f)
	{
		diff = DT * _moveSpeed * JUMPING_UP_SPEED_DIFF;
		playerLook = playerTransform->GetUp();
	}
	else
	{
		//_camera->GoLerp(false);
		// �ϴ� �浹 ����, y == 0 ����
		Vec3 pos = playerTransform->GetWorldPosition();
		pos.y = 0;
		playerTransform->SetWorldPosition(pos);
		_jumping = false;
		SetState(IDLE, false);
		return;
	}

	Vec3 playerPos = playerTransform->GetWorldPosition();
	playerLook.Normalize();
	Vec3 dist = playerLook * diff;
	playerPos -= dist;
	playerTransform->SetWorldPosition(playerPos);

	//_camera->UpdateCamera(true, diff, playerLook);
	SendMovePacket();


}

Vec3 Player::GetLandingPos()
{
	shared_ptr<Transform> transform = GetTransform();
	Vec3 currPos = transform->GetWorldPosition();
	currPos.y = 0;
	return currPos;
}

void Player::GetPosRotationLookFromMoveStat(const Protocol::S_Move& pkt, OUT Vec3& pos, OUT Vec3& angle, OUT Vec3& look)
{
	auto& movestat = pkt.movestat();
	pos.x = movestat.posx();
	pos.y = movestat.posy();
	pos.z = movestat.posz();

	angle.x = movestat.rotatex();
	angle.y = movestat.rotatey();
	angle.z = movestat.rotatez();

	look.x = movestat.lookx();
	look.y = movestat.looky();
	look.z = movestat.lookz();
}

void Player::GetObjectFromID(const Protocol::S_Move& pkt, OUT shared_ptr<ClientObject>& obj)
{
	uint64 id = pkt.id();
	obj = G_DevApp->FindObject(id);
}

void Player::Land(const Vec3 originCamPos, const Vec3 originCamAngle, const Vec3 originPlayerPos)
{
}


void Player::Fall()
{
	_info.mutable_movestat()->set_speed(_moveSpeed);

	shared_ptr<Transform> playersTransform = GetTransform();
	Vec3 playerPos = playersTransform->GetWorldPosition();
	Vec3 up = playersTransform->GetUp();
	up.Normalize();

	if (playerPos.y <= 1)
	{
		_info.mutable_movestat()->set_speed(_moveSpeed);

		_falling = false;
		_jumping = false;

		const Vec3& randPos = GetLandingPos();

		playersTransform->SetWorldPosition(randPos);

		//_camera->GoLerp(false);

		SetState(IDLE, true);

		SendMovePacket();

		return;
	}

	float delta = _moveSpeed * DT;
	Vec3 diff = up * delta;
	playerPos -= diff;
	playersTransform->SetWorldPosition(playerPos);

	//_camera->UpdateCamera(true, delta, up);

	SendMovePacket();
}


void Player::SetOtherObjState(shared_ptr<ClientObject> obj, ObjectState state)
{
	obj->SetState(state, false);
}



