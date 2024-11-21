#pragma once
#include "IExecute.h"
#include "NetworkLogger.h"

class GameObject;
class Shader;
class Mesh;
class Material;
class VertexBuffer;
class Player;
class Prop;
class ClientObject;
class Model;
class MyCamera;
class MyPlayer;
class ObjectCreator;


enum
{
	MESH_RENDER = 0,
	MODEL_RENDER = 1,
	ANIM_RENDER = 2,
};

enum
{
	BOUND_DIST = 5,    // For Octree Search 
	OCTREE_SEARCH_DEFAULT_MIN_Y = 0,
	OCTREE_COLLISION_SEARCH_DEFAULT_MAX_Y = 1,
	OCTREE_FRUSTUM_SEARCH_DEFAULT_MAX_Y = 10,

};


class DevApp1 : public IExecute, public enable_shared_from_this<DevApp1>
{
public:
	DevApp1();
	~DevApp1();

	void Init() override;
	void Update() override;
	void Render() override;

	// Packetó��
	void MakeMyPlayer(const Protocol::S_MyPlayer& pkt);
	void HandleS_StartInfos(const Protocol::S_StartInfos& pkt);
	void HandleAddObject(const Protocol::S_AddObject& pkt);
	void HandleS_RemoveObject(const Protocol::S_RemoveObject& pkt);
	void HandleS_Move(const Protocol::S_Move& pkt);
	void HandleS_ChangeState(const Protocol::S_ChangeState& pkt);
	void HandleS_ChangeDir(const Protocol::S_ChangeDir& pkt);
	void HandleS_Skill(const Protocol::S_Skill& pkt);
	void HandleS_SpecialSkill(const Protocol::S_SpecialSkill& pkt);
	void HandleS_CreateProjectile(const Protocol::S_CreateProjectile& pkt);

	// TODO : 충돌 체크를 엔진단으로 옮기는 게 좋겠음
	void CheckCollision(shared_ptr<ClientObject> player, Vec3 moveDir, OUT bool& collided);

	// Getter
	shared_ptr<ClientObject> FindObject(uint64 id);
	bool IsMyPlayer(uint64 id);
	shared_ptr<MyPlayer> GetMyPlayer();
	uint64 GetMyPlayerID();
	shared_ptr<class Projectile> GetSphereBall(shared_ptr<GameObject> player);
	
	
	// Setter
	void SetTargetMarkOn(const Vec3& pos);
	void SetSphereBallOn(shared_ptr<GameObject> sphereBall);
	void SetTargetMarkOff();
	void SetSphereBallOff(shared_ptr<GameObject> sphereBall);
	
	void AddTemporalObject(uint64 id, shared_ptr<GameObject> obj);
	void DeleteTemporalObject(uint64 id);
	shared_ptr<GameObject> GetTemporalObject(uint64 id);
	Matrix GetLookCameraMatrix(shared_ptr<GameObject> obj);

private:

	shared_ptr<GameObject> CreateContainer(const ObjectInfo& info);
	shared_ptr<GameObject> CreateTower(const ObjectInfo& info);
	shared_ptr<GameObject> CreateSmallMonster(const ObjectInfo& info);
	shared_ptr<GameObject> CreateLargeMonster(const ObjectInfo& info);
	shared_ptr<GameObject> CreateProjectile(const ObjectInfo& info);

private:
	// For Tests
	void CreateManyCubes();
	

private:
	shared_ptr<Shader> _shader;
	ObjectCreator* _objectCreator = nullptr;
	shared_ptr<class Scene> _devScene = nullptr;
	unordered_map<uint64, shared_ptr<GameObject>> _temporalObjects;

	
public:
	unique_ptr<NetworkLogger> _logger = nullptr;
};

extern shared_ptr<DevApp1> G_DevApp;