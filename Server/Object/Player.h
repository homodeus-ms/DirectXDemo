#pragma once
#include "GameObject.h"

class Player : public GameObject
{
	using Super = GameObject;

public:
	Player();
	virtual ~Player();
	void SetSession(GameSessionRef& session) { _session = session; }
	GameSessionRef GetSession() { return _session.lock(); }


private:
	weak_ptr<GameSession> _session;

};