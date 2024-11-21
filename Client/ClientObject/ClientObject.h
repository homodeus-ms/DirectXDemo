#pragma once
#include "CharacterMoveScript.h"

class GameObject;

enum
{
	ANIM_IDLE = 0,
	ANIM_MOVE = 1,
	ANIM_SKILL = 2,
	ANIM_SPECIAL_SKILL = 3,
};


class ClientObject : public GameObject
{
	using Super = GameObject;
public:
	ClientObject(ObjectType objectType);
	virtual ~ClientObject();

	virtual void Awake() override;
	virtual void Update() override;
	virtual void UpdateIdle() {};
	virtual void UpdateMove() {};
	virtual void UpdateJump() {};
	virtual void UpdateSkill() {};
	virtual void UpdateSpecialSkill() {};

	virtual void RotateY(uint64 id, bool toLeft, bool isMyMove) {}
	void DoSync();

	// Packet Send Helper 함수들
	void SendChangeState();
	void SendMovePacket();
	void SendChangeDirPacket(float yaw);
	void SendSkillPacket(bool isSpecialSkill, uint64 timeStampForSpecialSkill = 0);
	void SendCreateProjectilePacket(uint64 ownerId, ProjectileType type, Vec3 pos, Vec3 dir);

	void RotateYaw(float yaw);

	// Getter
	Protocol::ObjectInfo& GetInfo() { return _info; }
	ObjectType GetObjectType();
	uint64 GetID() { return _info.objectid(); }
	ObjectState GetState() { return _info.state(); }
	shared_ptr<CharacterMoveScript> GetScript() { return _script; }
	Vec3 GetWorldPos() { return { _info.movestat().posx(), _info.movestat().posy() , _info.movestat().posz() }; }
	bool HasCollided() { return _info.movestat().collided(); }
	class BoundingCube GetCollisionBoundingCube();
	

	// Setter
	void SetMoveScript(shared_ptr<CharacterMoveScript> script) { _script = script; }
	void SetInfo(const Protocol::ObjectInfo& info, bool skipScale = false);
	virtual void SetState(ObjectState state, bool sendPacket);

	void UpdateInfo();
	void UpdateMoveInfo(const Protocol::S_Move& pkt);
	void SyncTransformPosWithInfo();
	void SetCollide(bool collide) { _info.mutable_movestat()->set_collided(collide); }
	void SetSkillReady();
	
protected:
	// Consts
	const float ROTATE_SPEED = 2.f;
	const uint32 SPECIAL_SKILL_START_FRAME = 40;
	

protected:

	ObjectType _objectType;
	Protocol::ObjectInfo _info;
	shared_ptr<CharacterMoveScript> _script;

	float _moveSpeed;
	float _rotateSpeed = ROTATE_SPEED;

	bool _dirtyFlag = false;

	uint64 _skillStart;
	bool _skillEnd = true;
};

