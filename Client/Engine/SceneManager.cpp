#include "pch.h"
#include "SceneManager.h"

void SceneManager::Update()
{
	if (_currScene == nullptr)
		return;

	_currScene->Update();
	_currScene->LateUpdate();
}
