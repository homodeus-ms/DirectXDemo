#include "pch.h"
#include "CharacterMoveScript.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModelAnimator.h"
#include "Camera.h"
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "ClientObject.h"

#include "DevApp1.h"
#include "ClientPacketHandler.h"
#include "NetworkManager.h"

CharacterMoveScript::CharacterMoveScript(float moveSpeed, float rotateSpeed)
	: _moveSpeed(moveSpeed), _rotateSpeed(rotateSpeed)
{
	
}

CharacterMoveScript::~CharacterMoveScript()
{
}

void CharacterMoveScript::Start()
{
	_camera = CUR_SCENE->GetCamera();
}

void CharacterMoveScript::Update()
{
	
	if (_cancelMove)
	{
		if (Owner()->GetState() == JUMP)
			MoveTo(true);
		else
			MoveTo();
		return;
	}
	if (_cameraLerp)
	{
		CameraLerp();
		return;
	}

	if (_jumpingState)
	{
		if (_fallingState)
			Fall();
		else
			Jump();
		return;
	}

	if (_camera == nullptr)
	{
		GetGameObject()->GetModelAnimator()->SetNextAnimIndex(ANIM_IDLE);
		return;
	}


	const shared_ptr<Transform> playerTransform = GetTransform();
	Vec3 playerPos = playerTransform->GetWorldPosition();
	const shared_ptr<Transform> cameraTransform = _camera->GetTransform();
	Vec3 cameraPos = cameraTransform->GetWorldPosition();

	float deltaTime = DT;

	
	auto animator = GetGameObject()->GetModelAnimator();

	if (INPUT->GetButton(KEY_TYPE::W))
	{
		if (animator->GetTweenDesc().currAnim.animIndex == ANIM_IDLE)
			animator->SetNextAnimIndex(ANIM_MOVE);

		Move(true);
	}
	else
	{
		animator->SetNextAnimIndex(ANIM_IDLE);
	}
	// 카메라의 회전
	if (INPUT->GetButton(KEY_TYPE::A))
	{
		//animator->SetNextAnimIndex(0);

		Vec3 toCameraVector = cameraPos - playerPos;

		Vec3 rotation = GetTransform()->GetLocalRotation();
		float d = DT * _rotateSpeed;
		rotation.y -= d;
		playerTransform->SetWorldRotation(rotation);

		Vec3 camRotation = cameraTransform->GetLocalRotation();
		camRotation.y -= d;
		cameraTransform->SetLocalRotation(camRotation);

		toCameraVector = Vec3::TransformNormal(toCameraVector, Matrix::CreateRotationY(-d));

		cameraPos = playerPos + toCameraVector;
		cameraTransform->SetWorldPosition(cameraPos);

	}
	if (INPUT->GetButton(KEY_TYPE::D))
	{
		//animator->SetNextAnimIndex(0);

		Vec3 toCameraVector = cameraPos - playerPos;

		Vec3 rotation = GetTransform()->GetLocalRotation();
		float d = DT * _rotateSpeed;
		rotation.y += d;
		playerTransform->SetWorldRotation(rotation);

		Vec3 camRotation = cameraTransform->GetLocalRotation();
		camRotation.y += d;
		cameraTransform->SetLocalRotation(camRotation);

		toCameraVector = Vec3::Transform(toCameraVector, Matrix::CreateRotationY(d));
		cameraPos = playerPos + toCameraVector;
		cameraTransform->SetWorldPosition(cameraPos);
	}
	else if (INPUT->GetButton(KEY_TYPE::SPACE))
	{
		_cameraLerp = true;
		_jumpingState = true;

		auto cameraTransform = _camera->GetTransform();

		_keepCameraPos = cameraPos;
		_keepCameraRotation = cameraTransform->GetLocalRotation();
		_keepPlayerPos = GetTransform()->GetWorldPosition();

		_cameraLerpTargetPos = GetCameraJumpPos();
		_cameraLerpTargetAngle = GetCameraJumpAngle();
	}
}



void CharacterMoveScript::LateUpdate()
{

}



void CharacterMoveScript::SetCameraPos()
{
	
}

void CharacterMoveScript::Move(bool myMove)
{
	Owner()->SetState(MOVE);
	auto playerTransform = GetTransform();
	Vec3 playerPos = playerTransform->GetWorldPosition();

	float diff = DT * _moveSpeed;

	Vec3 playerLook = playerTransform->GetLook();
	playerLook.Normalize();
	Vec3 dist = playerLook * diff;
	playerPos -= dist;
	playerTransform->SetWorldPosition(playerPos);

	if (myMove)
	{
		auto cameraTransform = _camera->GetTransform();
		Vec3 cameraPos = cameraTransform->GetWorldPosition();
		cameraPos -= dist;
		cameraTransform->SetWorldPosition(cameraPos);
		SendMovePacket();
	}

}

void CharacterMoveScript::CancelMove(const Protocol::S_Move& pkt)
{
	const Protocol::MoveStat& movestat = pkt.movestat();
	toPos = { movestat.posx(), movestat.posy(), movestat.posz() };
	toLook = { movestat.lookx(), movestat.looky(), movestat.lookz() };
	toRotate = { movestat.rotatex(), movestat.rotatey(), movestat.rotatez() };
	_cancelMove = true;
}

void CharacterMoveScript::MoveTo(bool isJumping)
{
	//Owner()->SetState(MOVE);
	auto animator = GetGameObject()->GetModelAnimator();
	animator->SetNextAnimIndex(ANIM_MOVE);

	auto transform = GetTransform();
	Vec3 currPos = transform->GetWorldPosition();

	if (abs((currPos - toPos).LengthSquared()) < 0.5f)
	{
		_cancelMove = false;
		transform->SetWorldPosition(toPos);
		auto info = Owner()->GetInfo();
		info.mutable_movestat()->set_collided(false);
		info.mutable_movestat()->set_state(IDLE);

		return;
	}

	Vec3 currRotate = transform->GetWorldRotation();

	Vec3 look = toPos - currPos;
	look.Normalize();
	float diff = DT * (isJumping ?  PLAYER_FALL_SPEED : _moveSpeed);
	currPos += look * diff;
	transform->SetWorldPosition(currPos);

	if (_camera != nullptr)
	{
		Vec3 cameraPos = _camera->GetTransform()->GetWorldPosition();
		cameraPos += look * diff;
		_camera->GetTransform()->SetWorldPosition(cameraPos);
	}

}

void CharacterMoveScript::SendMovePacket()
{
	shared_ptr<ClientObject> owner = static_pointer_cast<ClientObject>(_gameObject.lock());
	auto transform = owner->GetTransform();
	
	Vec3 pos = transform->GetWorldPosition();
	Vec3 look = transform->GetLook();
	Vec3 rotate = transform->GetWorldRotation();
	float speed = owner->GetInfo().movestat().speed();

	Protocol::C_TryMove pkt;
	Protocol::MoveStat* moveStat = pkt.mutable_movestat();

	{
		pkt.set_id(owner->GetID());
		moveStat->set_state(Owner()->GetState());
		moveStat->set_posx(pos.x);
		moveStat->set_posy(pos.y);
		moveStat->set_posz(pos.z);
		moveStat->set_lookx(look.x);
		moveStat->set_looky(look.y);
		moveStat->set_lookz(look.z);
		moveStat->set_rotatex(rotate.x);
		moveStat->set_rotatey(rotate.y);
		moveStat->set_rotatez(rotate.z);
		moveStat->set_speed(speed);
	}
	
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);

}

void CharacterMoveScript::Jump()
{
	Owner()->GetInfo().mutable_movestat()->set_state(JUMP);
	
	shared_ptr<Transform> playersTransform = GetTransform();
	shared_ptr<Transform> camerasTransform = _camera->GetTransform();

	Vec3 playerPos = playersTransform->GetWorldPosition();
	Vec3 up = playersTransform->GetUp();
	Vec3 cameraPos = camerasTransform->GetWorldPosition();
	Vec3 cameraRotate = camerasTransform->GetLocalRotation();

	if (playerPos.y >= 50)
	{
		_fallingState = true;
		_cameraLerpTargetPos = _keepCameraPos;
		_cameraLerpTargetAngle = _keepCameraRotation;
		return;
	}

	const float speed = _moveSpeed;
	Vec3 diff = up * PLAYER_JUMP_UP_SPEED * DT;
	playerPos += diff;
	cameraPos += diff;

	playersTransform->SetWorldPosition(playerPos);
	camerasTransform->SetWorldPosition(cameraPos);

	SendMovePacket();
}

void CharacterMoveScript::Fall()
{
	shared_ptr<Transform> playersTransform = GetTransform();
	shared_ptr<Transform> camerasTransform = _camera->GetTransform();

	Vec3 playerPos = playersTransform->GetWorldPosition();
	Vec3 up = playersTransform->GetUp();
	Vec3 cameraPos = camerasTransform->GetWorldPosition();
	Vec3 cameraRotate = camerasTransform->GetLocalRotation();

	if (playerPos.y < 1)
	{
		_fallingState = false;
		_jumpingState = false;

		playersTransform->SetWorldPosition(_keepPlayerPos);

		_cameraLerp = true;
		//Owner()->SetState(IDLE);

		SendMovePacket();
		
		return;
	}

	const float speed = _moveSpeed;
	Vec3 diff = up * PLAYER_FALL_SPEED * DT;
	playerPos -= diff;
	cameraPos -= diff;

	playersTransform->SetWorldPosition(playerPos);
	camerasTransform->SetWorldPosition(cameraPos);

	SendMovePacket();
}

void CharacterMoveScript::CameraLerp()
{

	auto transform = _camera->GetTransform();
	Vec3 currPos = transform->GetWorldPosition();

	if ((_cameraLerpTargetPos - currPos).LengthSquared() < 0.1f)
	{
		transform->SetWorldPosition(_cameraLerpTargetPos);
		transform->SetLocalRotation(_cameraLerpTargetAngle);
		_cameraLerp = false;
		return;
	}
	Vec3 dir = _cameraLerpTargetPos - currPos;
	dir.Normalize();
	currPos += dir * DT * CAMERA_LERP_SPEED;
	transform->SetWorldPosition(currPos);
	
	Vec3 rotate = transform->GetLocalRotation();
	if (abs(_cameraLerpTargetAngle.x - rotate.x) > 0.01)
	{
		float x = DT * 2.4;
		if (_cameraLerpTargetAngle.x < rotate.x)
			x *= -1;
		transform->SetLocalRotation(Vec3(rotate.x + x, rotate.y, 0));
	}
}

Vec3 CharacterMoveScript::GetCameraJumpPos()
{
	auto cameraTransform = _camera->GetTransform();
	Vec3 pos = cameraTransform->GetWorldPosition();

	pos = GetTransform()->GetWorldPosition();    // 플레이어 위치
	Vec3 look = GetTransform()->GetLook();
	pos.y += CAMERA_JUMP_POS_Y_OFFSET;
	pos += look * CAMERA_JUMP_POS_Z_OFFSET;

	return pos;
}

Vec3 CharacterMoveScript::GetCameraJumpAngle()
{
	auto currRotation = _camera->GetTransform()->GetLocalRotation();
	return Vec3(XM_PI / 2 - 0.2f, currRotation.y, 0);
}




