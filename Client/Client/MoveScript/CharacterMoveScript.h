#pragma once
#include "MonoBehaviour.h"

class Player;

enum
{
	ANIM_IDLE = 0,
	ANIM_MOVE = 1,
	ANIM_SKILL = 2,
};

class CharacterMoveScript : public MonoBehaviour
{
public:

	CharacterMoveScript(float moveSpeed, float rotateSpeed);
	virtual ~CharacterMoveScript();
	
	virtual void Start() override;
	virtual void Update() override;
	virtual void LateUpdate() override;

	void Move(bool myMove = true);
	
	void CancelMove(const Protocol::S_Move& pkt);
	void MoveTo(bool isJumping = false);

	// Setter
	void SetOwner(shared_ptr<Player> player) { _owner = player; }
	

	// Getter
	shared_ptr<Player> Owner() { return _owner.lock(); }
	

private:
	


private:
	void SetCameraPos();
	void SendMovePacket();
	void Jump();
	void Fall();
	void CameraLerp();
	Vec3 GetCameraJumpPos();
	Vec3 GetCameraJumpAngle();


private:

	friend class DevApp1;

	enum
	{
		CAMERA_LERP_SPEED = 20,
		CAMERA_JUMP_POS_Z_OFFSET = 200,
		CAMERA_JUMP_POS_Y_OFFSET = 8,

		PLAYER_JUMP_UP_SPEED = 1800,
		PLAYER_FALL_SPEED = 2700,
	};

	float _moveSpeed;
	float _rotateSpeed;


	shared_ptr<class GameObject> _camera;
	weak_ptr<Player> _owner;
	
	// CancelMove°ü·Ã
	Vec3 toPos;
	Vec3 toLook;
	Vec3 toRotate;

	bool _cancelMove = false;
	bool _jumpingState = false;
	bool _fallingState = false;
	bool _cameraLerp = false;

	Vec3 _cameraLerpTargetPos;
	Vec3 _cameraLerpTargetAngle;

	Vec3 _keepPlayerPos;
	Vec3 _keepCameraPos;
	Vec3 _keepCameraRotation;

	
};

