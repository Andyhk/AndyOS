#include "Game.h"
#include "GEngine.h"
#include "stdio.h"

Game::Game()
{
	active_cam = 0;
}

void Game::AddObject(GameObject* object)
{
	objects.Add(object);
}

void Game::AddCamera(Camera* cam)
{
	cameras.Add(cam);

	if (!active_cam)
		SetActiveCamera(cam);
}

void Game::AddLightSource(LightSource* light)
{
	lights.Add(light);
}

Camera* Game::GetActiveCamera()
{
	return active_cam;
}

void Game::SetActiveCamera(Camera* cam)
{
	active_cam = cam;
}

GameObject* Game::GetObject(String name)
{
	for (int i = 0; i < objects.Count(); i++)
	{
		printf(objects[i]->GetName().ToChar());

		GameObject* obj = objects[i];
		if (obj->GetName() == name)
			return obj;
	}

	return 0;
}

Camera* Game::GetCamera(String name)
{
	for (int i = 0; i < cameras.Count(); i++)
	{
		Camera* cam = cameras[i];
		if (cam->GetName() == name)
			return cam;
	}

	return 0;
}

LightSource* Game::GetLightSource(String name)
{
	for (int i = 0; i < lights.Count(); i++)
	{
		LightSource* light = lights[i];
		if (light->GetName() == name)
			return light;
	}

	return 0;
}

