#include "pch.h"
#include "Main.h"
#include "Game.h"
//#include "..\\DrawingPractices\\MovingDemo.h"
#include "DevApp1.h"

shared_ptr<DevApp1> G_DevApp;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	GameDesc desc;
	desc.appName = L"3DRenderer";
	desc.hInstance = hInstance;
	desc.vsync = false;
	desc.hWnd = NULL;
	desc.width = 1080;
	desc.height = 810;
	desc.clearColor = Color(0.f, 0.f, 0.f, 0.f);
	auto app = make_shared<DevApp1>();
	desc.app = app;

	G_DevApp = app;

	GAME->Run(desc);

	return 0;
}