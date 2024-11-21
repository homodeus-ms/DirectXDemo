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
		//SetState(IDLE, false);
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

//void Player::Move()
//{
//	
//}

//void Player::MoveTo(uint64 id, bool isJumping)
//{
//	shared_ptr<ModelAnimator> animator;
//	shared_ptr<Transform> transform;
//	
//	if (id = GetID())
//	{
//		animator = GetModelAnimator();
//		transform = GetTransform();
//	}
//	else
//	{
//		shared_ptr<ClientObject> obj = G_DevApp->FindObject(id);
//		animator = obj->GetModelAnimator();
//		transform = obj->GetTransform();
//	}
//	
//	assert(animator != nullptr);
//	assert(transform != nullptr);
//
//	animator->SetNextAnimIndex(ANIM_MOVE);
//	Vec3 currPos = transform->GetWorldPosition();
//
//	
//	if (abs((currPos - _syncPos).LengthSquared()) < 0.5f)
//	{
//		auto info = GetInfo();
//		info.mutable_movestat()->set_collided(false);
//		info.set_state(IDLE);
//
//		return;
//	}
//
//	Vec3 currRotate = transform->GetWorldRotation();
//
//	Vec3 look = _syncPos - currPos;
//	look.Normalize();
//	float diff = DT * _moveSpeed;
//	currPos += look * diff;
//	transform->SetWorldPosition(currPos);
//
//
//	if (id == GetID())
//	{
//		Vec3 cameraPos = _camera->GetTransform()->GetWorldPosition();
//		cameraPos += look * diff;
//		_camera->GetTransform()->SetWorldPosition(cameraPos);
//	}
//}

/*
void Player::MoveOtherObjectTo()
{
	if (_shouldPopNext)
	{
		_shouldPopNext = false;
		const auto& pkt = _syncQueue.front();
		GetPosRotationLookFromMoveStat(pkt, _syncPos, _syncAngle, _syncLook);
		GetObjectFromID(pkt, _syncObj);
		_syncObj->SetState(pkt.state(), false);
	}

	auto objTr = _syncObj->GetTransform();
	Vec3 objPos = objTr->GetWorldPosition();
	Vec3 objAngle = objTr->GetWorldRotation();
	Vec3 objLook = objTr->GetLook();
	objLook.Normalize();

	float posDiff = (objPos - _syncPos).LengthSquared();
	objTr->SetLocalRotation(_syncAngle);

	objTr->SetWorldPosition(_syncPos);

	_syncQueue.pop();
	_shouldPopNext = true;

}

void Player::CancelMove(const Protocol::S_Move& pkt)
{
	const Protocol::MoveStat& movestat = pkt.movestat();
	*_info.mutable_movestat() = movestat;

	auto transform = GetTransform();
	Vec3 toPos = { movestat.posx(), movestat.posy(), movestat.posz() };
	transform->SetWorldPosition(toPos);
	transform->SetWorldRotation({ movestat.rotatex(), movestat.rotatey(), movestat.rotatez() });

	Vec3 look = transform->GetLook();
	look.Normalize();

	_camera->UpdateCamera(toPos, look);
}

void Player::MoveOtherObject(const Protocol::S_Move& pkt)
{
	int64 id = pkt.id();
	auto obj = G_DevApp->FindObject(id);

	SetOtherObjState(obj, pkt.state());

	auto objTr = obj->GetTransform();
	Vec3 objPos = objTr->GetWorldPosition();
	Vec3 objAngle = objTr->GetWorldRotation();
	Vec3 objLook = objTr->GetLook();
	objLook.Normalize();

	Vec3 targetPos;
	Vec3 targetLook;
	Vec3 targetAngle;

	GetPosRotationLookFromMoveStat(pkt, targetPos, targetAngle, targetLook);
	
	objTr->SetLocalRotation(targetAngle);
	objTr->SetWorldPosition(targetPos);
}
*/


void Player::RotateY(uint64 id, bool toLeft, bool isMyMove)
{
	
}

void Player::Jump()
{
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
		// 일단 충돌 무시, y == 0 으로
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



