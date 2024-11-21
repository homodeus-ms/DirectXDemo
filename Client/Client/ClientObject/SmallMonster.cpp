#include "pch.h"
#include "SmallMonster.h"
#include "Transform.h"

SmallMonster::SmallMonster()
	: Super(ObjectType::OBJECT_TYPE_SMALL_MONSTER)
{

}
SmallMonster::~SmallMonster()
{

}

void SmallMonster::Update()
{
	Vec3 rotation = GetTransform()->GetLocalRotation();
	float yaw = rotation.y + DT * ROTATION_SPEED;
	RotateYaw(yaw);
}
