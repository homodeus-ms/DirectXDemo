#pragma once
#include "MonoBehaviour.h"

class ClientObject;
class MyPlayer;

class CharacterMoveScript : public MonoBehaviour
{
public:

	CharacterMoveScript();
	virtual ~CharacterMoveScript();
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	
	bool IsKeyPressingValid(float dt);

	// Setter
	void SetOwner(shared_ptr<ClientObject> player);
	void SetKeyDownTime() { _firstKeyDownTime = 0.f; }

	// Getter
	shared_ptr<MyPlayer> GetOwner() { return _owner.lock(); }
	
private:
	
	
private:

	friend class DevApp1;

	const float KEY_PRESSING_THRESHOLD = 0.1f;

	weak_ptr<MyPlayer> _owner;
	int32 _lastMousePosX;
	bool _mouseRbuttonDown = false;
	KEY_TYPE _currMovingKey = KEY_TYPE::NONE;

	float _firstKeyDownTime = -1.f;
	
};

