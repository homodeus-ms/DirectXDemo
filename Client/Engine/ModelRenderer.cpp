#include "pch.h"
#include "ModelRenderer.h"
#include "Shader.h"
#include "Model.h"
#include "ModelMesh.h"
#include "Material.h"
#include "Transform.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Light.h"
#include "Camera.h"
#include "GameObject.h"

ModelRenderer::ModelRenderer(shared_ptr<Shader> shader)
	: Super(ComponentType::ModelRenderer), _shader(shader)
{
}

ModelRenderer::~ModelRenderer()
{
}


void ModelRenderer::SetModel(shared_ptr<Model> model)
{
	_model = model;

	const auto& materials = _model->GetMaterials();
	for (auto& material : materials)
	{
		material->SetShader(_shader);
	}
}

void ModelRenderer::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer)
{
	if (_model == nullptr)
		return;

	// VP DATA
	_shader->PushVPDataToGPU(Camera::S_MatView, Camera::S_MatProjection);
	// Light
	auto mainLight = SCENE->GetCurrentScene()->GetLight();
	if (mainLight)
		_shader->PushLightDataToGPU(mainLight->GetLight()->GetLightDesc());

	// Bone, 현재 인스턴싱되는 물체들이 동일한 계층구조를 가지고 있다고 가정
	BoneDesc boneDesc;
	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; ++i)
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

InstanceID ModelRenderer::GetInstanceID()
{
	return make_pair(reinterpret_cast<uint64>(_model.get()), reinterpret_cast<uint64>(_shader.get()));
}
