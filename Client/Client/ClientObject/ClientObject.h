#pragma once
#include "C:\kiwi\3DRenderer\Libraries\Include\Engine\GameObject.h"

#include "CharacterMoveScript.h"



class ClientObject : public GameObject
{
public:
	ClientObject(ObjectType objectType);
	virtual ~ClientObject();


	ObjectType GetObjectType();
	Protocol::ObjectInfo& GetInfo() { return _info; }
	void SetInfo(const Protocol::ObjectInfo& info) { _info = info; }

	void CancelMove(const Protocol::S_Move& pkt);
	void Move(uint64 id, Protocol::MoveStat& moveStat);

	// Getter
	uint64 GetID() { return _info.objectid(); }
	ObjectState GetState() { return _info.movestat().state(); }

	// Setter
	void SetMoveScript(shared_ptr<CharacterMoveScript> script) { _script = script; }
	shared_ptr<CharacterMoveScript> GetScript() { return _script; }
	void SetState(ObjectState state);
	void SetMoveStat(const Protocol::MoveStat& stat) { auto m = _info.mutable_movestat(); *m = stat; }

protected:
	ObjectType _objectType;
	Protocol::ObjectInfo _info;
	shared_ptr<CharacterMoveScript> _script;

};

