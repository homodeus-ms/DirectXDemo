#pragma once
#include "Scene.h"

class SceneManager
{
	DECLARE_SINGLE(SceneManager);

public:
	void Update();

	template<typename T>
	void ChangeScene(shared_ptr<T> scene)
	{
		_currScene = scene;
		scene->Start();
	}

	shared_ptr<Scene> GetCurrentScene() { return _currScene; }

private:
	shared_ptr<Scene> _currScene = make_shared<Scene>();

};

