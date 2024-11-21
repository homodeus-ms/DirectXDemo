#include "pch.h"
#include "CharacterMoveScript.h"
#include "Transform.h"
#include "GameObject.h"
#include "ModelAnimator.h"
#include "Camera.h"
#include "Scene.h"
#include "GameObject.h"
#include "Player.h"
#include "MyPlayer.h"
#include "ClientObject.h"
#include "DevApp1.h"
#include "ClientPacketHandler.h"
#include "NetworkManager.h"

CharacterMoveScript::CharacterMoveScript()
{
	
}

CharacterMoveScript::~CharacterMoveScript()
{
}

void CharacterMoveScript::Start()
{
	
}

void CharacterMoveScript::Update()
{
	// Left Click Picking
	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		const POINT point = INPUT->GetMousePos();
		GetOwner()->Pick(point);
	}


	// Mouse�� ī�޶� ���� ȸ��

	if (INPUT->GetButtonDown(KEY_TYPE::RBUTTON))
	{
		_mouseRbuttonDown = true;
		_lastMousePosX = INPUT->GetMousePos().x;
	}
	else if (INPUT->GetButtonUp(KEY_TYPE::RBUTTON))
	{
		_mouseRbuttonDown = false;
	}

	if (_mouseRbuttonDown)
	{
		int32 currMouseX = INPUT->GetMousePos().x;
		int32 diff = _lastMousePosX - currMouseX;

		GetOwner()->UpdateCameraRotate(diff);

		if (_currMovingKey != KEY_TYPE::NONE)
		{
			GetOwner()->SetMoveDir(_currMovingKey);
		}

		_lastMousePosX = currMouseX;
	}

	// Change Dir

	if (INPUT->GetButtonDown(KEY_TYPE::W))
	{
		GetOwner()->SetMoveDir(KEY_TYPE::W);
		SetKeyDownTime();
	}
	else if (INPUT->GetButtonDown(KEY_TYPE::A))
	{
		GetOwner()->SetMoveDir(KEY_TYPE::A);
		SetKeyDownTime();
	}
	else if (INPUT->GetButtonDown(KEY_TYPE::D))
	{
		GetOwner()->SetMoveDir(KEY_TYPE::D);
		SetKeyDownTime();
	}
	else if (INPUT->GetButtonDown(KEY_TYPE::S))
	{
		GetOwner()->SetMoveDir(KEY_TYPE::S);
		SetKeyDownTime();
	}

	// Move

	if (INPUT->GetButton(KEY_TYPE::W) && IsKeyPressingValid(DT))
	{
		if (IsKeyPressingValid(DT))
		{
			_currMovingKey = KEY_TYPE::W;
			GetOwner()->SetState(MOVE, true);
		}
	}
	else if (INPUT->GetButton(KEY_TYPE::A) && IsKeyPressingValid(DT))
	{
		if (IsKeyPressingValid(DT))
		{
			_currMovingKey = KEY_TYPE::A;
			GetOwner()->SetState(MOVE, true);
		}
	}
	else if (INPUT->GetButton(KEY_TYPE::D) && IsKeyPressingValid(DT))
	{
		if (IsKeyPressingValid(DT))
		{
			_currMovingKey = KEY_TYPE::D;
			GetOwner()->SetState(MOVE, true);
		}
	}
	else if (INPUT->GetButton(KEY_TYPE::S))
	{
		if (IsKeyPressingValid(DT))
		{
			_currMovingKey = KEY_TYPE::S;
			GetOwner()->SetState(MOVE, true);
		}
	}
	else
	{
		if (_currMovingKey != KEY_TYPE::NONE)
		{
			GetOwner()->SendLastMovePacket();
		}
		GetOwner()->SetState(IDLE, true);
		_currMovingKey = KEY_TYPE::NONE;
		GetOwner()->SetCollide(false);
	}

	// Attack

	if (INPUT->GetButton(KEY_TYPE::F))
	{
		GetOwner()->SetSendSkillPacketReady();
		GetOwner()->SetState(SKILL, true);
		
	}
	if (INPUT->GetButton(KEY_TYPE::G))
	{
		GetOwner()->SpecialAttack();
	}

	if (INPUT->GetButton(KEY_TYPE::SPACE))
	{
		if (GetOwner()->CanJump())
			GetOwner()->Jump();
	}
}



void CharacterMoveScript::LateUpdate()
{

}

bool CharacterMoveScript::IsKeyPressingValid(float dt)
{
	_firstKeyDownTime += dt;
	
	return _firstKeyDownTime > KEY_PRESSING_THRESHOLD;
}



void CharacterMoveScript::SetOwner(shared_ptr<ClientObject> player)
{
	_owner = static_pointer_cast<MyPlayer>(player);
}







