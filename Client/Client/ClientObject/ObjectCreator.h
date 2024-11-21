#pragma once

class GameObject;

class ObjectCreator
{
public:
	ObjectCreator(shared_ptr<Shader> shader);
	~ObjectCreator() {}
	
	shared_ptr<GameObject> CreateCamera(shared_ptr<class MyPlayer> player);
	shared_ptr<GameObject> CreateLight();
	shared_ptr<GameObject> CreatePlayer(const ObjectInfo& info, bool isMyPlayer);
	shared_ptr<GameObject> CreateContainer(const ObjectInfo& info);
	shared_ptr<GameObject> CreateTower(const ObjectInfo& info);
	shared_ptr<GameObject> CreateSmallMonster(const ObjectInfo& info);
	shared_ptr<GameObject> CreateLargeMonster(const ObjectInfo& info);
	shared_ptr<GameObject> CreateProjectile(const ObjectInfo& info);
	void CreateTargetMarker();
	shared_ptr<class Projectile> CreateSphereBall(shared_ptr<GameObject> player);

	// Getter
	shared_ptr<GameObject> GetTargetMarker();
	shared_ptr<class Projectile> GetSphereBall(shared_ptr<GameObject> player);
	shared_ptr<class Projectile> GetSphereBall(Vec3 pos, Vec3 dir);

private:
	// consts
	friend class DevApp1;

	const float ANIM_FRAME_SPEED = 2.5f;
	const uint64 MY_CAMERA_ID = 0xFFFF'FFFF;     // TEMP : 하위 32비트만
	const uint64 GLOBAL_LIGHT_ID = 0xFFFF'FFFE;  // TEMP : 하위 32비트만
	const float SPHERE_BALL_RADIUS = 0.5f;

	// read only
	Vec3 PLAYER_EXTENTS;
	Vec3 CONTAINER_EXTENTS;
	Vec3 TOWER_EXTENTS;
	Vec3 LARGE_MONSTER_EXTENTS;
	float SMALL_MONSTER_RADIUS;

private:
	shared_ptr<Shader> _shader;
	shared_ptr<class Model> _containerModel;
	shared_ptr<class Model> _towerModel;
	shared_ptr<class Model> _kachujin;


	shared_ptr<class Material> _smallMonsterMaterial;
	shared_ptr<class Material> _targetMaterial;
	shared_ptr<class Material> _sphereBallMaterial;
	
	shared_ptr<GameObject> _targetMarker;
	shared_ptr<GameObject> _sphereBall;
};

