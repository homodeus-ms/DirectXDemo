#include "pch.h"
#include "Shader.h"
#include "ModelAnimator.h"
#include "Material.h"
#include "ModelMesh.h"
#include "Model.h"
#include "ModelAnimation.h"
#include "Texture.h"
#include "Transform.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Camera.h"
#include "GameObject.h"

ModelAnimator::ModelAnimator(shared_ptr<Shader> shader)
	: Super(ComponentType::Animator), _shader(shader)
{

	_tweenDesc.nextAnim.animIndex = 0;
	//_tweenDesc.tweenSumTime = 3.f;

	
	// TEST
	//_tweenDesc.nextAnim.animIndex = rand() % 3;
	//_tweenDesc.tweenSumTime += rand() % 100;
}

ModelAnimator::~ModelAnimator()
{

}

#if 0
void ModelAnimator::Update()
{
	if (_model == nullptr)
		return;
	if (_texture == nullptr)
		CreateTexture();

	_keyframeDesc.sumTime += DT;

	shared_ptr<ModelAnimation> current = _model->GetAnimationByIndex(_keyframeDesc.animIndex);
	if (current)
	{
		float timePerFrame = 1 / (current->frameRate * _keyframeDesc.speed);
		if (_keyframeDesc.sumTime >= timePerFrame)
		{
			_keyframeDesc.sumTime = 0.f;
			_keyframeDesc.currFrame = (_keyframeDesc.currFrame + 1) % current->frameCount;
			_keyframeDesc.nextFrame = (_keyframeDesc.currFrame + 1) % current->frameCount;
		}

		_keyframeDesc.ratio = (_keyframeDesc.sumTime / timePerFrame);
	}

	// Anim Update
	ImGui::InputInt("AnimIndex", &_keyframeDesc.animIndex);
	_keyframeDesc.animIndex %= _model->GetAnimationCount();
	ImGui::InputInt("CurrFrame", (int*)&_keyframeDesc.currFrame);
	_keyframeDesc.currFrame %= _model->GetAnimationByIndex(_keyframeDesc.animIndex)->frameCount;
	ImGui::InputFloat("Speed", &_keyframeDesc.speed, 0.5f, 4.f);

	// 애니메이션 현재 프레임 정보
	CBUFFER_MANAGER->PushKeyframeDataToGPU(_keyframeDesc);

	// SRV를 통해 정보 전달
	_shader->GetSRV("TransformMap")->SetResource(_srv.Get());

	// Bones
	BoneDesc boneDesc;

	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.tranforms[i] = bone->transform;
	}
	CBUFFER_MANAGER->PushBoneDataToGPU(boneDesc);

	// Transform
	auto world = GetTransform()->GetWorldMatrix();
	CBUFFER_MANAGER->PushTransformDataToGPU(TransformDesc{ world });

	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
			mesh->material->Update();

		// BoneIndex
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		uint32 stride = mesh->vertexBuffer->GetStride();
		uint32 offset = mesh->vertexBuffer->GetOffset();

		DC->IASetVertexBuffers(0, 1, mesh->vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
		DC->IASetIndexBuffer(mesh->indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

		_shader->DrawIndexed(0, _pass, mesh->indexBuffer->GetCount(), 0, 0);
	}
}

void ModelAnimator::UpdateTweenData()
{
}

void ModelAnimator::Update()
{
	if (_model == nullptr)
		return;
	if (_texture == nullptr)
		CreateTexture();

	TweenDesc& desc = _tweenDesc;

	desc.currAnim.sumTime += DT;

	// 현재 애니메이션
	{
		shared_ptr<ModelAnimation> currentAnim = _model->GetAnimationByIndex(desc.currAnim.animIndex);
		if (currentAnim)
		{
			float timePerFrame = 1 / (currentAnim->frameRate * desc.currAnim.speed);
			if (desc.currAnim.sumTime >= timePerFrame)
			{
				desc.currAnim.sumTime = 0;
				desc.currAnim.currFrame = (desc.currAnim.currFrame + 1) % currentAnim->frameCount;
				desc.currAnim.nextFrame = (desc.currAnim.currFrame + 1) % currentAnim->frameCount;
			}

			desc.currAnim.ratio = (desc.currAnim.sumTime / timePerFrame);
		}
	}

	// 다음 애니메이션이 존재하면
	if (desc.nextAnim.animIndex >= 0)
	{
		desc.tweenSumTime += DT;
		// 시간 경과에 따라 보간 비율을 구해줌
		desc.tweenRatio = desc.tweenSumTime / desc.tweenDuration;

		if (desc.tweenRatio >= 1.f)
		{
			// 애니메이션 교체
			desc.currAnim = desc.nextAnim;
			desc.ClearNextAnim();
		}
		else
		{
			// 교체중
			shared_ptr<ModelAnimation> nextAnim = _model->GetAnimationByIndex(desc.nextAnim.animIndex);
			desc.nextAnim.sumTime += DT;

			float timePerFrame = 1.f / (nextAnim->frameRate * desc.nextAnim.speed);

			if (desc.nextAnim.ratio >= 1.f)
			{
				desc.nextAnim.sumTime = 0;

				desc.nextAnim.currFrame = (desc.nextAnim.currFrame + 1) % nextAnim->frameCount;
				desc.nextAnim.nextFrame = (desc.nextAnim.currFrame + 1) % nextAnim->frameCount;
			}			 

			desc.nextAnim.ratio = desc.nextAnim.sumTime / timePerFrame;
		}
	}

	// Anim Update
	ImGui::InputInt("AnimIndex", &desc.currAnim.animIndex);
	_keyframeDesc.animIndex %= _model->GetAnimationCount();

	static int32 nextAnimIndex = 0;
	if (ImGui::InputInt("NextAnimIndex", &nextAnimIndex))
	{
		nextAnimIndex %= _model->GetAnimationCount();
		desc.ClearNextAnim(); // 기존꺼 밀어주기
		desc.nextAnim.animIndex = nextAnimIndex;
	}

	if (_model->GetAnimationCount() > 0)
		desc.currAnim.animIndex %= _model->GetAnimationCount();

	ImGui::InputFloat("Speed", &desc.currAnim.speed, 0.5f, 4.f);

	CBUFFER_MANAGER->PushTweenDataToGPU(desc);

	// SRV를 통해 정보 전달
	_shader->GetSRV("TransformMap")->SetResource(_srv.Get());

	// Bones
	BoneDesc boneDesc;

	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.tranforms[i] = bone->transform;
	}
	CBUFFER_MANAGER->PushBoneDataToGPU(boneDesc);

	// Transform
	auto world = GetTransform()->GetWorldMatrix();
	CBUFFER_MANAGER->PushTransformDataToGPU(TransformDesc{ world });

	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
			mesh->material->Update();

		// BoneIndex
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		uint32 stride = mesh->vertexBuffer->GetStride();
		uint32 offset = mesh->vertexBuffer->GetOffset();

		DC->IASetVertexBuffers(0, 1, mesh->vertexBuffer->GetComPtr().GetAddressOf(), &stride, &offset);
		DC->IASetIndexBuffer(mesh->indexBuffer->GetComPtr().Get(), DXGI_FORMAT_R32_UINT, 0);

		_shader->DrawIndexed(0, _pass, mesh->indexBuffer->GetCount(), 0, 0);
	}
}
#endif

void ModelAnimator::Update()
{

}

void ModelAnimator::UpdateTweenData()
{

	TweenDesc& desc = _tweenDesc;

	desc.currAnim.sumTime += DT;

	// 현재 애니메이션
	{
		shared_ptr<ModelAnimation> currentAnim = _model->GetAnimationByIndex(desc.currAnim.animIndex);
		if (currentAnim)
		{
			float timePerFrame = 1 / (currentAnim->frameRate * desc.currAnim.speed);
			if (desc.currAnim.sumTime >= timePerFrame)
			{
				desc.currAnim.sumTime = 0;
				desc.currAnim.currFrame = (desc.currAnim.currFrame + 1) % currentAnim->frameCount;
				desc.currAnim.nextFrame = (desc.currAnim.currFrame + 1) % currentAnim->frameCount;
			}

			desc.currAnim.ratio = (desc.currAnim.sumTime / timePerFrame);
		}
	}

	// 다음 애니메이션이 존재하면
	if (desc.nextAnim.animIndex >= 0)
	{
		desc.tweenSumTime += DT;
		// 시간 경과에 따라 보간 비율을 구해줌
		desc.tweenRatio = desc.tweenSumTime / desc.tweenDuration;

		if (desc.tweenRatio >= 1.f)
		{
			// 애니메이션 교체
			desc.currAnim = desc.nextAnim;
			desc.ClearNextAnim();
		}
		else
		{
			// 교체중
			shared_ptr<ModelAnimation> nextAnim = _model->GetAnimationByIndex(desc.nextAnim.animIndex);
			desc.nextAnim.sumTime += DT;

			float timePerFrame = 1.f / (nextAnim->frameRate * desc.nextAnim.speed);

			if (desc.nextAnim.ratio >= 1.f)
			{
				desc.nextAnim.sumTime = 0;

				desc.nextAnim.currFrame = (desc.nextAnim.currFrame + 1) % nextAnim->frameCount;
				desc.nextAnim.nextFrame = (desc.nextAnim.currFrame + 1) % nextAnim->frameCount;
			}

			desc.nextAnim.ratio = desc.nextAnim.sumTime / timePerFrame;
		}
	}
}

void ModelAnimator::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer)
{
	if (_model == nullptr)
		return;
	if (_texture == nullptr)
		CreateTexture();


	// VP DATA
	_shader->PushVPDataToGPU(Camera::S_MatView, Camera::S_MatProjection);
	// Light
	auto mainLight = SCENE->GetCurrentScene()->GetLight();
	if (mainLight)
		_shader->PushLightDataToGPU(mainLight->GetLight()->GetLightDesc());


	// SRV를 통해 정보 전달
	_shader->GetSRV("TransformMap")->SetResource(_srv.Get());

	// Bones
	BoneDesc boneDesc;

	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
		boneDesc.tranforms[i] = bone->transform;
	}
	_shader->PushBoneDataToGPU(boneDesc);

	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
			mesh->material->Update();

		// BoneIndex
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		mesh->vertexBuffer->PushDataToGPU();
		mesh->indexBuffer->PushDataToGPU();
		buffer->PushDataToGPU();

		_shader->DrawIndexedInstanced(0, _renderPass, mesh->indexBuffer->GetCount(), buffer->GetCount());
		
	}
}

void ModelAnimator::SetModel(shared_ptr<Model> model)
{
	_model = model;

	const auto& materials = _model->GetMaterials();
	for (auto& material : materials)
	{
		material->SetShader(_shader);
	}
}

InstanceID ModelAnimator::GetInstanceID()
{
	return make_pair(reinterpret_cast<uint64>(_model.get()), reinterpret_cast<uint64>(_shader.get()));
}

void ModelAnimator::CreateTexture()
{
	if (_model->GetAnimationCount() == 0)
		return;

	_animTransforms.resize(_model->GetAnimationCount());
	for (uint32 i = 0; i < _model->GetAnimationCount(); i++)
		CreateAnimationTransform(i);

	// Creature Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;    // Matrix는 4 * 16 = 64바이트이므로
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = _model->GetAnimationCount();
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 16바이트
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		const uint32 dataSize = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
		const uint32 pageSize = dataSize * MAX_MODEL_KEYFRAMES;
		void* mallocPtr = ::malloc(pageSize * _model->GetAnimationCount());

		// 파편화된 데이터를 조립한다.
		for (uint32 c = 0; c < _model->GetAnimationCount(); c++)
		{
			uint32 startOffset = c * pageSize;

			BYTE* pageStartPtr = reinterpret_cast<BYTE*>(mallocPtr) + startOffset;

			for (uint32 f = 0; f < MAX_MODEL_KEYFRAMES; f++)
			{
				void* ptr = pageStartPtr + dataSize * f;
				::memcpy(ptr, _animTransforms[c].transforms[f].data(), dataSize);
			}
		}

		// 리소스 만들기
		vector<D3D11_SUBRESOURCE_DATA> subResources(_model->GetAnimationCount());

		for (uint32 c = 0; c < _model->GetAnimationCount(); c++)
		{
			void* ptr = (BYTE*)mallocPtr + c * pageSize;
			subResources[c].pSysMem = ptr;
			subResources[c].SysMemPitch = dataSize;
			subResources[c].SysMemSlicePitch = pageSize;
		}

		HRESULT hr = DEVICE->CreateTexture2D(&desc, subResources.data(), _texture.GetAddressOf());
		CHECK(hr);

		::free(mallocPtr);
	}

	// Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = _model->GetAnimationCount();

		HRESULT hr = DEVICE->CreateShaderResourceView(_texture.Get(), &desc, _srv.GetAddressOf());
		CHECK(hr);
	}
}

void ModelAnimator::CreateAnimationTransform(uint32 index)
{
	vector<Matrix> cacheMatsToAnimGlobalRoot(MAX_MODEL_TRANSFORMS, Matrix::Identity);

	shared_ptr<ModelAnimation> animation = _model->GetAnimationByIndex(index);

	for (uint32 f = 0; f < animation->frameCount; f++)
	{
		for (uint32 b = 0; b < _model->GetBoneCount(); b++)
		{
			shared_ptr<ModelBone> bone = _model->GetBoneByIndex(b);

			Matrix matAnimation;

			shared_ptr<ModelKeyframe> frame = animation->GetKeyframe(bone->name);
			if (frame != nullptr)
			{
				ModelKeyframeData& data = frame->transforms[f];

				Matrix S, R, T;
				S = Matrix::CreateScale(data.scale.x, data.scale.y, data.scale.z);
				R = Matrix::CreateFromQuaternion(data.rotation);
				T = Matrix::CreateTranslation(data.translation.x, data.translation.y, data.translation.z);

				// 로컬 -> 부모
				matAnimation = S * R * T;
			}
			else
			{
				matAnimation = Matrix::Identity;
			}

			
			//*********************//
			// ex) Anim1 -> Anim2  //
			//*********************//

			// Anim1의 Global -> Local Matrix
			Matrix toGlobalRootMatrix = bone->transform;
			Matrix invGlobal = toGlobalRootMatrix.Invert();

			int32 parentIndex = bone->parentIndex;
			Matrix matParentToGlobalRoot = Matrix::Identity;
			if (parentIndex >= 0)    // 부모가 있음
				matParentToGlobalRoot = cacheMatsToAnimGlobalRoot[parentIndex];

			// Anim2의 local -> Global Matrix
			cacheMatsToAnimGlobalRoot[b] = matAnimation * matParentToGlobalRoot;

			// 최종
			// Anim1의 Global -> Anim1의 Local -> Anim2의 Global로 변환해주는 Matrix
			_animTransforms[index].transforms[f][b] = invGlobal * cacheMatsToAnimGlobalRoot[b];
		}
	}
}
