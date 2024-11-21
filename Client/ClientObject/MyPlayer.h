#include "Player.h"

class MyCamera;
class Projectile;

class MyPlayer : public Player
{
	using Super = Player;

public:
	MyPlayer();
	virtual ~MyPlayer();

	virtual void Awake() override;
	virtual void Update() override;
	virtual void UpdateIdle() override;
	virtual void UpdateMove() override;
	virtual void UpdateJump() override;
	virtual void UpdateSkill() override;
	virtual void UpdateSpecialSkill() override;
	virtual void LateUpdate() override;

	void HandleCollided(const Protocol::S_Move& pkt);
	void UpdateNextMovePos();
	void SendLastMovePacket();
	void SpecialAttack();

	void Pick(POINT point);
	void DisappearShootBall();
	void UpdateCameraRotate(int32 xDiff);

	// Getter
	float GetYaw() { return _yaw; }
	Vec3 GetMoveDir() { return _moveDir; }
	shared_ptr<MyCamera> GetMyCamera() { return _camera; }
	shared_ptr<Projectile> GetSphereBall() { return _sphereBall; }

	// Setter
	void SetSendSkillPacketReady();
	void SetCamera(shared_ptr<MyCamera> camera) { _camera = camera; }
	void SetMoveDir(KEY_TYPE keyTYpe);

private:
	// consts
	const uint64 SEND_TERM = 200;
	const float TARGET_REMAIN_TIME = 1000.f;
	const float SHOOT_BALL_DELAY = 0.4f;
	const float ROTATE_TO_TARGET_THRESHOLD = 0.984f;    // cos(10) to radian 

private:
	shared_ptr<MyCamera> _camera;

	Vec3 _moveDir;
	float _diff;
	float _yaw;
	// Move ��Ŷ ���� ����
	
	int32 _firstPackets = 5;
	uint64 _prevSendTime;
	
	// Attack����
	bool _bSkillPacketSent = false;

	Vec3 _targetPos;
	bool _bTargetOn = false;
	uint64 _targetOnStartTime = 0;

	shared_ptr<class Projectile> _sphereBall;
	Vec3 _sphereBallDir;
	bool _bShootBall = false;
	bool _bRotateToTarget = false;
	float _shootDelaySum = 0.f;
	
};
