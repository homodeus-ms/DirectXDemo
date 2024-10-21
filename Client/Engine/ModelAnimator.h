#pragma once
#include "Component.h"
#include "BindShaderDesc.h"

class Shader;
class Model;

struct AnimTransform
{
	// [ ][ ][ ][ ][ ][ ][ ] ... 250��
	// Bone �迭
	using TransformArrayType = array<Matrix, MAX_MODEL_TRANSFORMS>;
	// [ ][ ][ ][ ][ ][ ][ ] ... 500 ��
	// Frame������ Bone�迭
	array<TransformArrayType, MAX_MODEL_KEYFRAMES> transforms;
};

class ModelAnimator : public Component
{
	using Super = Component;

public:
	ModelAnimator(shared_ptr<Shader> shader);
	~ModelAnimator();

	virtual void Update() override;
	void UpdateTweenData();

	void SetModel(shared_ptr<Model> model);
	void SetRenderPass(uint8 pass) { _renderPass = pass; }
	shared_ptr<Shader> GetShader() { return _shader; }

	void RenderInstancing(shared_ptr<class InstancingBuffer>& buffer);
	InstanceID GetInstanceID();
	TweenDesc& GetTweenDesc() { return _tweenDesc; }

	void SetNextAnimIndex(int32 index) { _tweenDesc.nextAnim.animIndex = index; }

private:
	void CreateTexture();
	void CreateAnimationTransform(uint32 index);

private:
	vector<AnimTransform> _animTransforms;
	// shader�� cbuffer�� �Ѱ��� �� �����Ƿ�, srv�� ������
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _srv;

private:
	
	TweenDesc _tweenDesc;

private:
	shared_ptr<Shader>	_shader;
	uint8				_renderPass = 0;
	shared_ptr<Model>	_model;
};
