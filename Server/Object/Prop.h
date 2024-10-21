#pragma once
#include "GameObject.h"

class Prop : public GameObject
{
	using Super = GameObject;

public:
	Prop(int32 posX, int32 posZ);
	virtual ~Prop();

private:
	
};

