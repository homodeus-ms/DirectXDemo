#pragma once
#include "ClientObject.h"

class Monster : public ClientObject
{
	using Super = ClientObject;

public:
	Monster(ObjectType objectType);
	virtual ~Monster();

private:

};

