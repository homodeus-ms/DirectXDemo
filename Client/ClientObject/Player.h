#pragma once
#include "ClientObject.h"

class MyCamera;

struct AttackInfo
{
	int32 power = 5;
	int32 specialPower = 10;
	bool bSpecialValid = false;
	Vec3 specialSkillRange = { 5, 5, 5 };
	uint64 lastSpecialSkillUseTime = 0;
};

class Player : public ClientObject
{
	using Super = ClientObject;

public:
	Player();
	virtual ~Player() {}

	virtual void Update() override;
	virtual void UpdateIdle() override;
	virtual void UpdateMove() override;
	virtual void UpdateJump() override;
	virtual void UpdateSkill() override;
	virtual void UpdateSpecialSkill() override;

	virtual void RotateY(uint64 id, bool toLeft, bool isMyMove) override;

	void Jump();
	void Attack();
	void Land(const Vec3 originCamPos, const Vec3 originCamAngle, const Vec3 originPlayerPos);

	// Setter
	void FinishCameraLerp() { _jumping = true; }
	virtual void SetOtherObjState(shared_ptr<ClientObject> obj, ObjectState state);
	

	// Getter
	
	bool CanJump() const { return !_jumping; }
	AttackInfo GetAttackInfo() { return _attackInfo; }
	uint64 GetLastSpecialSkillUseTime() { return _attackInfo.lastSpecialSkillUseTime; }

	void SetLastSpecialSkillUseTime(uint64 now) { _attackInfo.lastSpecialSkillUseTime = now; }

private:
	void Fall();
	void DoJump();
	Vec3 GetLandingPos();
	void GetPosRotationLookFromMoveStat(const Protocol::S_Move& pkt, OUT Vec3& pos, OUT Vec3& angle, OUT Vec3& look);
	void GetObjectFromID(const Protocol::S_Move& pkt, OUT shared_ptr<ClientObject>& obj);

protected:
	// consts
	const uint32 SKILL_END_TICK = 560;
	const uint32 SPECIAL_SKILL_END_TICK = 920;
	const uint64 SPECIAL_SKILL_COOL_TIME = 3000;

private:

	AttackInfo _attackInfo;

	shared_ptr<ClientObject> _syncObj;
	Vec3 _syncLook;
	Vec3 _syncPos;
	Vec3 _syncAngle;

    // Jump 관련
	const float JUMPING_MOVE_SPEED_DIFF = 1.5f;
	const float JUMPING_UP_SPEED_DIFF = 3.5f;
	
	bool _jumping = false;
	bool _falling = false;

	uint64 _jumpStartTime;
	uint64 _jumpTime = 800;
	

};

