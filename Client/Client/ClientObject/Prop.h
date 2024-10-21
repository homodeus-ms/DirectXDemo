#pragma once
#include "ClientObject.h"
class Prop : public ClientObject
{
	using Super = ClientObject;
public:
	Prop(ObjectType objectType);
	virtual ~Prop() {}

private:
	
};

