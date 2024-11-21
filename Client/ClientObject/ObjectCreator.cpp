#include "pch.h"
#include "ObjectCreator.h"
#include "DevApp1.h"
#include "Model.h"
#include "ClientObject.h"
#include "Player.h"
#include "MyPlayer.h"
#include "Monster.h"
#include "SmallMonster.h"
#include "LargeMonster.h"
#include "Prop.h"
#include "Camera.h"
#include "Light.h"
#include "MyCamera.h"
#include "GlobalLight.h"
#include "Projectile.h"
#include "Transform.h"
#include "ModelAnimator.h"
#include "ModelRenderer.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "CharacterMoveScript.h"
#include "Mesh.h"
#include "Texture.h"
#include "OBBBoxCollider.h"
#include "SphereCollider.h"


ObjectCreator::ObjectCreator(shared_ptr<Shader> shader)
	: _shader(shader)
{
	// Kachujin 로드
	{
		_kachujin = make_shared<Model>();
		_kachujin->ReadModel(L"Kachujin/Kachujin");
		_kachujin->ReadMaterial(L"Kachujin/Kachujin");
		_kachujin->ReadAnimation(L"Kachujin/Idle");
		_kachujin->ReadAnimation(L"Kachujin/Run");
		_kachujin->ReadAnimation(L"Kachujin/Slash");    // Index번호 : ANIM_SKILL 단순공격
		_kachujin->ReadAnimation(L"Kachujin/Slash");    // Index번호 : ANIM_SPECIAL_SKILL 광역공격
	}

	// Prop Model 로드
	{
		_containerModel = make_shared<Model>();
		_containerModel->ReadModel(L"Container/Container");
		_containerModel->ReadMaterial(L"Container/Container");

		_towerModel = make_shared<Model>();
		_towerModel->ReadModel(L"Tower/Tower");
		_towerModel->ReadMaterial(L"Tower/Tower");
	}

	// SmallMonster 로드
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		shared_ptr<Texture> texture = RESOURCE->Load<Texture>(L"SmallMonster", L"..\\Resources\\Textures\\SmallMonster.png");
		material->SetDiffuseMap(texture);
		MaterialLightDesc& desc = material->GetMaterialLightDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);

		_smallMonsterMaterial = material;
		RESOURCE->Add(L"SmallMonster", material);
	}

	// Target Image 로드
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		shared_ptr<Texture> texture = RESOURCE->Load<Texture>(L"Target", L"..\\Resources\\Textures\\Target.png");
		material->SetDiffuseMap(texture);
		MaterialLightDesc& desc = material->GetMaterialLightDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);

		_targetMaterial = material;
		RESOURCE->Add(L"Target", material);

		CreateTargetMarker();
	}

	// SphereBall Material 로드
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		shared_ptr<Texture> texture = RESOURCE->Load<Texture>(L"FireBall", L"..\\Resources\\Textures\\FireBall.jpg");
		material->SetDiffuseMap(texture);
		MaterialLightDesc& desc = material->GetMaterialLightDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);

		_sphereBallMaterial = material;
		RESOURCE->Add(L"FireBall", material);

	}
}

shared_ptr<GameObject> ObjectCreator::CreateCamera(shared_ptr<MyPlayer> player)
{
	shared_ptr<MyCamera> camera = make_shared<MyCamera>();

	Vec3 playerPos = player->GetTransform()->GetWorldPosition();
	camera->GetTransform()->SetWorldPosition(playerPos + Vec3(0.f, 5.f, -10.f));
	camera->GetTransform()->SetWorldRotation(Vec3(0.3f, 0.f, 0.f));

	shared_ptr<Camera> componentCamera = make_shared<Camera>();
	camera->AddComponent(componentCamera);
	componentCamera->GetTransform()->SetWorldPosition(camera->GetTransform()->GetWorldPosition());
	componentCamera->GetTransform()->SetWorldRotation(camera->GetTransform()->GetWorldRotation());
	
	camera->GetInfo().set_objectid(MY_CAMERA_ID);

	player->SetCamera(camera);
	CUR_SCENE->SetMyCamera(static_pointer_cast<GameObject>(camera));
	camera->SetOwner(player);

	shared_ptr<GameObject> obj = static_pointer_cast<GameObject>(camera);

	CUR_SCENE->Add(camera->GetID(), obj);

	return obj;
}

shared_ptr<GameObject> ObjectCreator::CreateLight()
{
	shared_ptr<ClientObject> light = make_shared<GlobalLight>();
	light->AddComponent(make_shared<Light>());

	LightDesc desc;
	desc.ambient = Vec4(0.8f);
	desc.diffuse = Vec4(1.f);
	desc.specular = Vec4(1.f);
	desc.direction = Vec3(1.f, 0.f, 1.f);
	light->GetLight()->SetLightDesc(desc);

	light->GetInfo().set_objectid(GLOBAL_LIGHT_ID);

	shared_ptr<GameObject> obj = static_pointer_cast<GameObject>(light);

	CUR_SCENE->Add(light->GetID(), obj);

	return obj;
}

shared_ptr<GameObject> ObjectCreator::CreatePlayer(const ObjectInfo& info, bool isMyPlayer)
{
	shared_ptr<ClientObject> obj;
	if (isMyPlayer)
		obj = make_shared<MyPlayer>();
	else
		obj = make_shared<Player>();

	Protocol::MoveStat stat = info.movestat();

	obj->SetInfo(info);

	obj->AddComponent(make_shared<ModelAnimator>(_shader));
	obj->GetModelAnimator()->SetModel(_kachujin);
	obj->GetModelAnimator()->SetRenderPass(ANIM_RENDER);

	if (isMyPlayer)
	{
		auto script = make_shared<CharacterMoveScript>();
		obj->AddComponent(script);
		obj->SetMoveScript(script);
		script->SetOwner(obj);

		auto gameObject = static_pointer_cast<GameObject>(obj);
		CUR_SCENE->SetMyPlayer(gameObject);
	}

	auto animator = obj->GetModelAnimator();
	auto& desc = animator->GetTweenDesc();
	desc.currAnim.speed = ANIM_FRAME_SPEED;
	desc.nextAnim.speed = ANIM_FRAME_SPEED;

	// Collider
	{
		auto collider = make_shared<OBBBoxCollider>();
		collider->GetBoundingBox().Center = obj->GetWorldPos();
		collider->GetBoundingBox().Extents = PLAYER_EXTENTS;
		collider->SetGameObject(obj);
		obj->AddComponent(collider);
	}


	CUR_SCENE->Add(obj->GetID(), obj);

	obj->Awake();

	return obj;
}

shared_ptr<GameObject> ObjectCreator::CreateContainer(const ObjectInfo& info)
{
	shared_ptr<ClientObject> obj = make_shared<Prop>(ObjectType::OBJECT_TYPE_PROP_CONTAINER);

	Protocol::MoveStat stat = info.movestat();
	obj->SetInfo(info);

	obj->AddComponent(make_shared<ModelRenderer>(_shader));
	obj->GetModelRenderer()->SetModel(_containerModel);
	obj->GetModelRenderer()->SetRenderPass(MODEL_RENDER);

	// Collider
	{
		auto collider = make_shared<OBBBoxCollider>();
		collider->GetBoundingBox().Center = obj->GetWorldPos();
		collider->GetBoundingBox().Extents = CONTAINER_EXTENTS;
		collider->SetGameObject(obj);
		obj->AddComponent(collider);
	}

	CUR_SCENE->Add(obj->GetID(), obj);

	return static_pointer_cast<GameObject>(obj);
}

shared_ptr<GameObject> ObjectCreator::CreateTower(const ObjectInfo& info)
{
	shared_ptr<ClientObject> obj = make_shared<Prop>(ObjectType::OBJECT_TYPE_PROP_TOWER);

	Protocol::MoveStat stat = info.movestat();
	obj->SetInfo(info);

	obj->AddComponent(make_shared<ModelRenderer>(_shader));
	obj->GetModelRenderer()->SetModel(_towerModel);
	obj->GetModelRenderer()->SetRenderPass(MODEL_RENDER);

	// Collider
	{
		auto collider = make_shared<OBBBoxCollider>();
		collider->GetBoundingBox().Center = obj->GetWorldPos();
		collider->GetBoundingBox().Extents = TOWER_EXTENTS;
		collider->SetGameObject(obj);
		obj->AddComponent(collider);
	}

	CUR_SCENE->Add(obj->GetID(), obj);

	return static_pointer_cast<GameObject>(obj);
}

shared_ptr<GameObject> ObjectCreator::CreateSmallMonster(const ObjectInfo& info)
{
	shared_ptr<Mesh> mesh = RESOURCE->GetSphere();

	shared_ptr<ClientObject> obj = make_shared<SmallMonster>();
	obj->GetTransform()->SetLocalScale({ 1.5f, 1.5f, 1.5f });
	obj->SetInfo(info, true);
	obj->AddComponent(make_shared<MeshRenderer>(mesh, _smallMonsterMaterial));
	obj->GetMeshRenderer()->SetRenderPass(0);

	// Collider
	{
		auto collider = make_shared<SphereCollider>();
		collider->GetBoundingSphere().Center = obj->GetWorldPos();
		collider->GetBoundingSphere().Radius = SMALL_MONSTER_RADIUS;
		collider->SetGameObject(obj);
		obj->AddComponent(collider);
	}

	CUR_SCENE->Add(obj->GetID(), obj);

	return obj;
}

shared_ptr<GameObject> ObjectCreator::CreateLargeMonster(const ObjectInfo& info)
{
	return shared_ptr<GameObject>();
}

shared_ptr<GameObject> ObjectCreator::CreateProjectile(const ObjectInfo& info)
{
	return shared_ptr<GameObject>();
}

void ObjectCreator::CreateTargetMarker()
{
	shared_ptr<Mesh> mesh = RESOURCE->GetQuad();

	_targetMarker = make_shared<GameObject>();
	
	_targetMarker->AddComponent(make_shared<MeshRenderer>(mesh, _targetMaterial));
	_targetMarker->GetMeshRenderer()->SetRenderPass(0);

}

shared_ptr<Projectile> ObjectCreator::CreateSphereBall(shared_ptr<GameObject> player)
{
	shared_ptr<Mesh> mesh = RESOURCE->GetSphere();

	shared_ptr<Projectile> ball = make_shared<Projectile>();
	ball->GetTransform()->SetLocalScale({ 0.5f, 0.5f, 0.5f });
	ball->AddComponent(make_shared<MeshRenderer>(mesh, _sphereBallMaterial));
	ball->GetMeshRenderer()->SetRenderPass(0);
	{
		auto collider = make_shared<SphereCollider>();
		collider->GetBoundingSphere().Center = player->GetTransform()->GetWorldPosition();
		collider->GetBoundingSphere().Radius = SPHERE_BALL_RADIUS;
		collider->SetGameObject(ball);
		ball->AddComponent(collider);
	}

	return ball;
}

shared_ptr<GameObject> ObjectCreator::GetTargetMarker()
{
	return _targetMarker;
}

shared_ptr<class Projectile> ObjectCreator::GetSphereBall(shared_ptr<GameObject> player)
{
	return CreateSphereBall(player);
}

shared_ptr<class Projectile> ObjectCreator::GetSphereBall(Vec3 pos, Vec3 dir)
{
	shared_ptr<Mesh> mesh = RESOURCE->GetSphere();
	shared_ptr<Projectile> ball = make_shared<Projectile>();
	ball->GetTransform()->SetLocalScale({ 0.5f, 0.5f, 0.5f });
	ball->GetTransform()->SetWorldPosition(pos);
	ball->AddComponent(make_shared<MeshRenderer>(mesh, _sphereBallMaterial));
	ball->GetMeshRenderer()->SetRenderPass(0);

	ball->SetProjectileInfo(pos, dir);

	return ball;
}
