#include "pch.h"
#include "ClientObject.h"

ClientObject::ClientObject(ObjectType objectType)
    : _objectType(objectType)
{
}

ClientObject::~ClientObject()
{
}



ObjectType ClientObject::GetObjectType()
{
    uint64 id = _info.objectid();
    id >>= 32;
    return static_cast<ObjectType>(id);
}

void ClientObject::CancelMove(const Protocol::S_Move& pkt)
{
    assert(_script != nullptr);
    _script->CancelMove(pkt);
}

void ClientObject::Move(uint64 id, Protocol::MoveStat& moveStat)
{

}

void ClientObject::SetState(ObjectState state)
{
    _info.mutable_movestat()->set_state(state);
}