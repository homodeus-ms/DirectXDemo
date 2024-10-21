#pragma once
#include "IExecute.h"

class GameObject;
class Shader;
class Mesh;
class Material;
class VertexBuffer;
class Player;
class Prop;
class ClientObject;
class Model;

enum
{
	MESH_RENDER = 0,
	MODEL_RENDER = 1,
	ANIM_RENDER = 2,
};

const float ROTATE_SPEED = 1.f;
const float ANIM_FRAME_SPEED = 2.5f;
const float PLAYER_COLLIDER_RADIUS = 1.f;
const Vec3 CONTAINER_EXTENTS = { 2, 1, 3.5f };
const Vec3 TOWER_EXTENTS = { 2, 1, 2 };

class DevApp1 : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	// Packet√≥∏Æ
	void MakeMyPlayer(const Protocol::S_MyPlayer& pkt);
	void HandleAddObject(const Protocol::S_AddObject& pkt);
	void HandleS_Move(const Protocol::S_Move& pkt);

	// Getter
	shared_ptr<GameObject> FindObject(uint64 id);

private:

	void CreateCamera();
	void CreateLight();
	shared_ptr<Player> CreatePlayer(const ObjectInfo& info, bool myPlayer);
	shared_ptr<GameObject> CreateContainer(const ObjectInfo& info);
	shared_ptr<GameObject> CreateTower(const ObjectInfo& info);

	

private:
	// Tests
	void CreateManyCubes();

private:
	shared_ptr<Shader> _shader;
	
	shared_ptr<Player> _player;
	vector<shared_ptr<ClientObject>> _objects;
	shared_ptr<GameObject> _camera;
	shared_ptr<GameObject> _light;

	shared_ptr<Model> _containerModel;
	shared_ptr<Model> _towerModel;
};

extern shared_ptr<DevApp1> G_DevApp;