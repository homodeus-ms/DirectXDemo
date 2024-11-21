#include "pch.h"
#include "TimeManager.h"
#include "ClientObject.h"
#include "Transform.h"
#include "ModelAnimator.h"
#include "DevApp1.h"
#include "BoundingCube.h"

ClientObject::ClientObject(ObjectType objectType)
    : _objectType(objectType)
{
	
}

ClientObject::~ClientObject()
{
}

void ClientObject::Awake()
{
	Super::Awake();
}

void ClientObject::Update()
{
	Super::Update();

	switch (GetState())
	{
	case IDLE:
		UpdateIdle();
		break;
	case MOVE:
		UpdateMove();
		break;
	case JUMP:
		UpdateJump();
		break;
	case SKILL:
		UpdateSkill();
		break;
	case SPECIAL_SKILL:
		UpdateSpecialSkill();
		break;
	}

	if (_dirtyFlag)
	{
		DoSync();
		_dirtyFlag = false;
	}
	
}

ObjectType ClientObject::GetObjectType()
{
	return _objectType;
    /*uint64 id = _info.objectid();
    id >>= 32;
    return static_cast<ObjectType>(id);*/
}

void ClientObject::SetInfo(const Protocol::ObjectInfo& info, bool skipScale)
{
	_info = info;
	const Protocol::MoveStat& stat = _info.movestat();

	_moveSpeed = stat.speed();

	GetTransform()->SetWorldPosition(Vec3(stat.posx(), stat.posy(), stat.posz()));
	if (!skipScale)
		GetTransform()->SetWorldScale(Vec3(0.01f));

	Vec3 rotation = { stat.rotatex(), stat.rotatey(), stat.rotatez() };
	GetTransform()->SetWorldRotation(rotation);
}

void ClientObject::CancelMove(const Protocol::S_Move& pkt)
{
    assert(_script != nullptr);
    CancelMove(pkt);
}

void ClientObject::DoSync()
{
	SendChangeState();
}


void ClientObject::SendChangeState()
{
	Protocol::C_ChangeState pkt;
	pkt.set_id(GetID());
	pkt.set_state(GetState());
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
}

void ClientObject::SendMovePacket()
{
	Protocol::C_TryMove pkt;
	pkt.set_id(GetID());
	pkt.set_state(GetState());
	Protocol::MoveStat* moveStat = pkt.mutable_movestat();
	moveStat->CopyFrom(_info.movestat());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
}

void ClientObject::SendChangeDirPacket(float yaw)
{
	Protocol::C_ChangeDir pkt;
	pkt.set_id(GetID());
	pkt.set_yaw(yaw);
	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
}

void ClientObject::SendSkillPacket(bool isSpecialSkill, uint64 timeStampForSpecialSkill)
{
	if (isSpecialSkill)
	{
		Protocol::C_SpecialSkill pkt;
		pkt.set_id(GetID());
		pkt.set_timestamp(timeStampForSpecialSkill);
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	}
	else
	{
		Protocol::C_Skill pkt;
		pkt.set_id(GetID());
		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
		GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
	}
}

void ClientObject::SendCreateProjectilePacket(uint64 ownerId, ProjectileType type, Vec3 pos, Vec3 dir)
{
	Protocol::C_CreateProjectile pkt;
	pkt.set_ownerid(ownerId);
	pkt.set_type(type);
	Protocol::Vector3* pktPos = pkt.mutable_startpos();
	Protocol::Vector3* pktDir = pkt.mutable_dir();

	pktPos->set_x(pos.x);
	pktPos->set_y(pos.y);
	pktPos->set_z(pos.z);
	pktDir->set_x(dir.x);
	pktDir->set_y(dir.y);
	pktDir->set_z(dir.z);

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	GET_SINGLE(NetworkManager)->SendPacket(sendBuffer);
}

void ClientObject::RotateYaw(float yaw)
{
	GetTransform()->SetWorldRotation({0.f, yaw, 0.f});
	_info.mutable_movestat()->set_rotatey(yaw);
}


BoundingCube ClientObject::GetCollisionBoundingCube()
{
	Vec3 pos = GetWorldPos();

	BoundingCube cube({ pos.x - BOUND_DIST, OCTREE_SEARCH_DEFAULT_MIN_Y, pos.z - BOUND_DIST },
		{ pos.x + BOUND_DIST, OCTREE_COLLISION_SEARCH_DEFAULT_MAX_Y, pos.z + BOUND_DIST });

	return cube;
}

void ClientObject::SetState(ObjectState state, bool sendPacket)
{
	if (GetState() == SKILL || GetState() == SPECIAL_SKILL)
	{
		if (!_skillEnd)
			return;
	}

	if (state == GetState())
	{
		return;
	}

    _info.set_state(state);
	auto animator = GetModelAnimator();
	if (animator == nullptr)
		return;

	switch (state)
	{
	case IDLE:
		animator->SetNextAnimIndex(ANIM_IDLE);
		break;
	case MOVE:
		animator->SetNextAnimIndex(ANIM_MOVE);
		break;
	case SKILL:
		animator->SetNextAnimIndex(ANIM_SKILL);
		break;
	case SPECIAL_SKILL:
		animator->SetNextAnimIndex(ANIM_SPECIAL_SKILL);
		{
			KeyframeDesc& nextAnim = animator->GetTweenDesc().nextAnim;
			nextAnim.currFrame = SPECIAL_SKILL_START_FRAME;
		}
		break;
	}

	if (sendPacket)
		_dirtyFlag = true;
}

void ClientObject::UpdateInfo()
{
	shared_ptr<Transform> tr = GetTransform();
	Vec3 worldPos = tr->GetWorldPosition();
	Vec3 look = tr->GetLook();
	Vec3 rotation = tr->GetLocalRotation();

	Protocol::MoveStat* movestat = _info.mutable_movestat();

	movestat->set_posx(worldPos.x);
	movestat->set_posy(worldPos.y);
	movestat->set_posz(worldPos.z);

	movestat->set_lookx(look.x);
	movestat->set_looky(look.y);
	movestat->set_lookz(look.z);

	movestat->set_rotatex(rotation.x);
	movestat->set_rotatey(rotation.y);
	movestat->set_rotatez(rotation.z);
}

void ClientObject::UpdateMoveInfo(const Protocol::S_Move& pkt)
{
	Protocol::MoveStat* movestat = _info.mutable_movestat();
	movestat->CopyFrom(pkt.movestat());
}

void ClientObject::SyncTransformPosWithInfo()
{
	Protocol::MoveStat* movestat = _info.mutable_movestat();
	GetTransform()->SetWorldPosition({ movestat->posx(), movestat->posy(), movestat->posz() });
}

void ClientObject::SetSkillReady()
{
	_skillStart = ::GetTickCount64();
	_skillEnd = false;
}





