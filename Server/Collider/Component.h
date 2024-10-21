#pragma once

class GameObject;
class Transform;

enum class ComponentType : uint8
{
	Collider,
	End,
};
enum
{
	COMPONENT_COUNT = ComponentType::End,
};

class Component
{
public:
	Component(ComponentType type);
	virtual ~Component();

	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void FixedUpdate() {}

	ComponentType GetType() { return _type; }
	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();

private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }

	ComponentType _type;

protected:
	weak_ptr<GameObject> _gameObject;
};

