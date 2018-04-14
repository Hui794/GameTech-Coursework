#pragma once
#include <ncltech\Scene.h>

class TestScene : public Scene
{
public:
	TestScene(const std::string& friendly_name);
	virtual ~TestScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

	int score = 0;
	bool flag = false;
protected:
	void ShootABall();

	float m_AccumTime;
	GameObject* m_cube;

	Camera* camera;

private:
	//static int score;
};

/*
class TestScene : public Scene
{
public:
TestScene(const std::string& friendly_name);
virtual ~TestScene();

virtual void OnInitializeScene()	 override;
virtual void OnCleanupScene()		 override;
virtual void OnUpdateScene(float dt) override;

bool ExampleCallbackFunction1(PhysicsNode* self, PhysicsNode* collidingObject);
int score = 0;
bool flag =false;
protected:


float m_AccumTime;
GameObject* m_pPlayer;
GameObject*	m_Sphere;
GameObject* m_cube;

Camera* camera;

private:
//static int score;
};
*/