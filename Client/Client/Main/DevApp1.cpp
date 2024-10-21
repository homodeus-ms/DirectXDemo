#include "pch.h"
#include "DevApp1.h"
#include "Shader.h"
#include "GameObject.h"
#include "Camera.h"
#include "CameraScript.h"
#include "ModelRenderer.h"
#include "Material.h"
#include "Transform.h"
#include "Model.h"
#include "ModelAnimator.h"
#include "MeshRenderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Viewport.h"
#include "SphereCollider.h"
#include "Scene.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "Terrain.h"
#include "CharacterMoveScript.h"
#include "NetworkManager.h"
#include "Player.h"
#include "ClientObject.h"
#include "Prop.h"
#include "BoundingCube.h"

shared_ptr<GameObject> tempObj = nullptr;

void DevApp1::Init()
{
	Sleep(300);

	GET_SINGLE(NetworkManager)->Init();
	_shader = make_shared<Shader>(L"20. ViewportDemo.fx");

	// Prop Model 로드

	_containerModel = make_shared<Model>();
	_containerModel->ReadModel(L"Container/Container");
	_containerModel->ReadMaterial(L"Container/Container");

	_towerModel = make_shared<Model>();
	_towerModel->ReadModel(L"Tower/Tower");
	_towerModel->ReadMaterial(L"Tower/Tower");


	/*CreatePlayer();
	CreateCamera();
	CreateLight();*/
	
	//CreateContainers();
	//CreateTowers();

	//CreateManyCubes();

	

	
}

void DevApp1::Update()
{
	GET_SINGLE(NetworkManager)->Update();

	if (_camera != nullptr)
	{
		auto pt = _player->GetTransform();
		auto ct = _camera->GetTransform();
		Vec3 playerPos = pt->GetWorldPosition();
		Vec3 cameraPos = ct->GetWorldPosition();
		Vec3 angle = ct->GetLocalRotation();

		ImGui::Begin("CamDebug", nullptr);
		//ImGui::Text("%s : %f, %f, %f", "cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("%s : %f, %f, %f", "playerPos", playerPos.x, playerPos.y, playerPos.z);
		ImGui::Text("%s : %f, %f, %f", "cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("%s : %f, %f, %f", "cameraRotate", angle.x, angle.y, angle.z);

		
		auto script = _player->GetScript();
		Vec3 v = script->_keepCameraPos;
		ImGui::Text("%s : %f, %f, %f", "originCamPos", v.x, v.y, v.z);

		Vec3 OtherPlayerPos;
		for (auto& obj : _objects)
		{
			uint64 id = obj->GetID();
			if ((id >> 32) == ObjectType::OBJECT_TYPE_PLAYER && id != _player->GetID())
			{
				OtherPlayerPos = obj->GetTransform()->GetWorldPosition();
				break;
			}
		}
		
		ImGui::Text("%s : %f, %f, %f", "OtherPlayer", OtherPlayerPos.x, OtherPlayerPos.y, OtherPlayerPos.z);

		ImGui::End();
	}

}

void DevApp1::Render()
{
}

void DevApp1::MakeMyPlayer(const Protocol::S_MyPlayer& pkt)
{
	const ObjectInfo& info = pkt.info();

	_player = CreatePlayer(info, true);
	_player->SetInfo(info);
	CreateCamera();
	CreateLight();
	CUR_SCENE->Start();
}

void DevApp1::HandleAddObject(const Protocol::S_AddObject& pkt)
{
	uint64 myPlayerId = _player->GetInfo().objectid();
	const int32 objectCount = pkt.objects_size();

	for (int32 i = 0; i < objectCount; ++i)
	{
		const Protocol::ObjectInfo& info = pkt.objects(i);
		if (myPlayerId == info.objectid())
		{
			int a = 3;
			continue;
		}

		// [앞32비트 : 오브젝트타입   |   뒤32비트 : 오브젝트 아이디]
		uint64 id = info.objectid();
		ObjectType objectType = static_cast<ObjectType>((id >> 32));

		switch (objectType)
		{
		case ObjectType::OBJECT_TYPE_PLAYER:
		{
			shared_ptr<ClientObject> player = static_pointer_cast<ClientObject>(CreatePlayer(info, false));
			_objects.push_back(player);
			break;
		}
		case ObjectType::OBJECT_TYPE_PROP_CONTAINER:
		{
			CreateContainer(info);
			break;
		}
		case ObjectType::OBJECT_TYPE_PROP_TOWER:
		{
			CreateTower(info);
			break;
		}
		}
	}
	
}

void DevApp1::HandleS_Move(const Protocol::S_Move& pkt)
{
	uint64 id = pkt.id();
	ObjectType objectType = static_cast<ObjectType>((id >> 32));

	if (_player->GetID() == id)
	{
		if (pkt.movestat().collided() == true)
		{
			_player->CancelMove(pkt);
			
		}
		return;
	}

	shared_ptr<ClientObject> movedPlayer = static_pointer_cast<ClientObject>(FindObject(id));
	if (movedPlayer == nullptr)
	{
		assert(false);
		return;
	}

	movedPlayer->GetScript()->CancelMove(pkt);
}

shared_ptr<GameObject> DevApp1::FindObject(uint64 id)
{
	for (auto& object : _objects)
	{
		if (object->GetID() == id)
		{
			return object;
		}
	}
	return nullptr;
}



void DevApp1::CreateCamera()
{
	auto camera = make_shared<GameObject>();
	
	assert(_player != nullptr);
	Vec3 playerPos = _player->GetTransform()->GetWorldPosition();

	camera->GetTransform()->SetWorldPosition(playerPos + Vec3(0.f, 5.f, -10.f));
	camera->GetTransform()->SetWorldRotation(Vec3(0.3f, 0.f, 0.f));
	camera->AddComponent(make_shared<Camera>());
	//camera->AddComponent(make_shared<CameraScript>());

	_camera = camera;

	CUR_SCENE->Add(camera);
}

void DevApp1::CreateLight()
{
	auto light = make_shared<GameObject>();
	light->AddComponent(make_shared<Light>());

	LightDesc desc;
	desc.ambient = Vec4(0.8f);
	desc.diffuse = Vec4(1.f);
	desc.specular = Vec4(1.f);
	desc.direction = Vec3(1.f, 0.f, 1.f);
	light->GetLight()->SetLightDesc(desc);

	_light = light;

	CUR_SCENE->Add(light);
}

shared_ptr<Player> DevApp1::CreatePlayer(const ObjectInfo& info, bool myPlayer)
{
	// Animation Character
	shared_ptr<Model> m1 = make_shared<Model>();
	m1->ReadModel(L"Kachujin/Kachujin");
	m1->ReadMaterial(L"Kachujin/Kachujin");
	m1->ReadAnimation(L"Kachujin/Idle");
	m1->ReadAnimation(L"Kachujin/Run");
	m1->ReadAnimation(L"Kachujin/Slash");

	Protocol::MoveStat stat = info.movestat();

	auto obj = make_shared<Player>();

	obj->SetInfo(info);
	obj->GetTransform()->SetWorldPosition(Vec3(stat.posx(), stat.posy(), stat.posz()));
	obj->GetTransform()->SetWorldScale(Vec3(0.01f));
	obj->GetTransform()->SetLocalRotation(Vec3(stat.rotatex(), stat.rotatey(), stat.rotatez()));

	obj->AddComponent(make_shared<ModelAnimator>(_shader));

	obj->GetModelAnimator()->SetModel(m1);
	obj->GetModelAnimator()->SetRenderPass(ANIM_RENDER);

	auto script = make_shared<CharacterMoveScript>(stat.speed(), ROTATE_SPEED);
	obj->AddComponent(script);
	obj->SetMoveScript(script);
	script->SetOwner(obj);

	if (myPlayer)
	{
		

		auto gameObject = static_pointer_cast<GameObject>(obj);
		CUR_SCENE->SetPlayer(gameObject);
	}

	auto animator = obj->GetModelAnimator();
	auto& desc = animator->GetTweenDesc();
	desc.currAnim.speed = ANIM_FRAME_SPEED;
	desc.nextAnim.speed = ANIM_FRAME_SPEED;

	CUR_SCENE->Add(obj);

	return obj;
}

shared_ptr<GameObject> DevApp1::CreateContainer(const ObjectInfo& info)
{
	auto obj = make_shared<Prop>(ObjectType::OBJECT_TYPE_PROP_CONTAINER);

	Protocol::MoveStat stat = info.movestat();
	obj->SetInfo(info);

	obj->GetTransform()->SetWorldPosition(Vec3(stat.posx(), stat.posy(), stat.posz()));
	obj->GetTransform()->SetWorldScale(Vec3(0.01f));
	obj->GetTransform()->SetLocalRotation(Vec3(stat.rotatex(), stat.rotatey(), stat.rotatez()));

	obj->AddComponent(make_shared<ModelRenderer>(_shader));
	obj->GetModelRenderer()->SetModel(_containerModel);
	obj->GetModelRenderer()->SetRenderPass(MODEL_RENDER);

	CUR_SCENE->Add(obj);


	_objects.push_back(static_pointer_cast<ClientObject>(obj));

	return obj;
}

shared_ptr<GameObject> DevApp1::CreateTower(const ObjectInfo& info)
{
	auto obj = make_shared<Prop>(ObjectType::OBJECT_TYPE_PROP_TOWER);

	Protocol::MoveStat stat = info.movestat();
	obj->SetInfo(info);

	obj->GetTransform()->SetWorldPosition(Vec3(stat.posx(), stat.posy(), stat.posz()));
	obj->GetTransform()->SetWorldScale(Vec3(0.01f));
	obj->GetTransform()->SetLocalRotation(Vec3(stat.rotatex(), stat.rotatey(), stat.rotatez()));

	obj->AddComponent(make_shared<ModelRenderer>(_shader));
	obj->GetModelRenderer()->SetModel(_towerModel);
	obj->GetModelRenderer()->SetRenderPass(MODEL_RENDER);

	CUR_SCENE->Add(obj);

	
	_objects.push_back(static_pointer_cast<ClientObject>(obj));

	return obj;
}



void DevApp1::CreateManyCubes()
{

	for (int i = 0; i < 4000; ++i)
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		auto texture = RESOURCE->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		material->SetDiffuseMap(texture);
		MaterialLightDesc& desc = material->GetMaterialLightDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);
		RESOURCE->Add(L"Veigar", material);

		auto mesh = RESOURCE->GetBox();

		auto obj = make_shared<GameObject>();
		
		obj->GetTransform()->SetLocalPosition(Vec3(rand() % 2000 - 1000, 0.f, rand() % 2000 - 1000));
		obj->AddComponent(make_shared<MeshRenderer>(mesh, material));
		obj->GetMeshRenderer()->SetRenderPass(0);


		CUR_SCENE->Add(obj);
	}
}

