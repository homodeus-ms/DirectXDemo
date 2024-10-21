#include "pch.h"
#include "InstancingManager.h"
#include "Shader.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "ModelAnimator.h"
#include "ModelRenderer.h"
#include "Transform.h"

void InstancingManager::Render(vector<shared_ptr<GameObject>>& gameObjects)
{
	ClearData();

	RenderMeshRenderer(gameObjects);
	RenderModelRenderer(gameObjects);
	RenderAnimRenderer(gameObjects);
}

void InstancingManager::ClearData()
{
	for (auto& pair : _bufferMap)
	{
		shared_ptr<InstancingBuffer>& buffer = pair.second;
		buffer->ClearData();
	}
}

void InstancingManager::RenderMeshRenderer(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	// 분류작업
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr)
			continue;

		const InstanceID instanceId = gameObject->GetMeshRenderer()->GetInstanceID();
		cache[instanceId].push_back(gameObject);
	}

	for (auto& pair : cache)
	{
		const vector<shared_ptr<GameObject>>& objects = pair.second;

		const InstanceID instanceId = pair.first;

		for (int32 i = 0; i < objects.size(); ++i)
		{
			const shared_ptr<GameObject>& gameObject = objects[i];
			InstancingData data;
			data.world = gameObject->GetTransform()->GetWorldMatrix();

			AddData(instanceId, data);
		}

		// 같은 녀석들끼리 넣어줬으니 이제 그려주기
		shared_ptr<InstancingBuffer>& buffer = _bufferMap[instanceId];
		objects[0]->GetMeshRenderer()->RenderInstancing(buffer);
	}
}

void InstancingManager::RenderModelRenderer(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	// 분류작업
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetModelRenderer() == nullptr)
			continue;

		const InstanceID instanceId = gameObject->GetModelRenderer()->GetInstanceID();
		cache[instanceId].push_back(gameObject);
	}

	for (auto& pair : cache)
	{
		const vector<shared_ptr<GameObject>>& objects = pair.second;

		const InstanceID instanceId = pair.first;

		for (int32 i = 0; i < objects.size(); ++i)
		{
			const shared_ptr<GameObject>& gameObject = objects[i];
			InstancingData data;
			data.world = gameObject->GetTransform()->GetWorldMatrix();

			AddData(instanceId, data);
		}

		// 같은 녀석들끼리 넣어줬으니 이제 그려주기
		shared_ptr<InstancingBuffer>& buffer = _bufferMap[instanceId];
		objects[0]->GetModelRenderer()->RenderInstancing(buffer);
	}
}

void InstancingManager::RenderAnimRenderer(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	// 분류작업
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		if (gameObject->GetModelAnimator() == nullptr)
			continue;

		const InstanceID instanceId = gameObject->GetModelAnimator()->GetInstanceID();
		cache[instanceId].push_back(gameObject);
	}

	for (auto& pair : cache)
	{
		shared_ptr<InstancedTweenDesc> tweenDesc = make_shared<InstancedTweenDesc>();

		const vector<shared_ptr<GameObject>>& objects = pair.second;

		const InstanceID instanceId = pair.first;

		for (int32 i = 0; i < objects.size(); ++i)
		{
			const shared_ptr<GameObject>& gameObject = objects[i];
			InstancingData data;
			data.world = gameObject->GetTransform()->GetWorldMatrix();

			AddData(instanceId, data);

			// INSTANCING
			gameObject->GetModelAnimator()->UpdateTweenData();
			TweenDesc& desc = gameObject->GetModelAnimator()->GetTweenDesc();
			tweenDesc->tweens[i] = desc;
		}

		objects[0]->GetModelAnimator()->GetShader()->PushTweenDataToGPU(*tweenDesc.get());

		// 같은 녀석들끼리 넣어줬으니 이제 그려주기
		shared_ptr<InstancingBuffer>& buffer = _bufferMap[instanceId];
		objects[0]->GetModelAnimator()->RenderInstancing(buffer);
		
	}
}

void InstancingManager::AddData(InstanceID instanceId, InstancingData& data)
{
	if (_bufferMap.find(instanceId) == _bufferMap.end())
		_bufferMap[instanceId] = make_shared<InstancingBuffer>();

	_bufferMap[instanceId]->AddData(data);
}
