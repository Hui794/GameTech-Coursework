#include "Scene1.h"

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\CommonMeshes.h>
#include "ncltech\SphereCollisionShape.h"
#include "ncltech\CuboidCollisionShape.h"
using namespace CommonUtils;

const float SpeedFactor = 30;

TestScene::TestScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
{
	m_cube = new GameObject();
}

TestScene::~TestScene()
{

}


void TestScene::OnInitializeScene()
{
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(true);

	//Set the camera position
	//GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	//GraphicsPipeline::Instance()->GetCamera()->SetYaw(140.f);
	//GraphicsPipeline::Instance()->GetCamera()->SetPitch(-20.f);

	camera = GraphicsPipeline::Instance()->GetCamera();
	camera->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	camera->SetYaw(140.f);
	camera->SetPitch(-20.f);
	m_AccumTime = 0.0f;
	NCLDebug::Log("This is a log entry - It will printed to the console, on screen log and <project_dir>\program_output.txt");
	//NCLERROR("THIS IS AN ERROR!"); // <- On Debug mode this will also trigger a breakpoint in your code!



	//<--- SCENE CREATION --->
	//Create Ground
	this->AddGameObject(BuildCuboidObject("Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(10.0f, 1.0f, 10.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	//ÊúÏò
	this->AddGameObject(BuildCuboidObject("Ground",
		Vector3(-10.0f, 2.0f, 0.0f),
		Vector3(0.5f, 2.0f, 10.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	this->AddGameObject(BuildCuboidObject("Ground",
		Vector3(10.0f, 2.0f, 0.0f),
		Vector3(0.5f, 2.0f, 10.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 0.5f)));
	//ºáÏò
	this->AddGameObject(BuildCuboidObject("Ground",
		Vector3(0.0f, 2.0f, 10.0f),
		Vector3(10.0f, 2.0f, 0.5f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	this->AddGameObject(BuildCuboidObject("Ground",
		Vector3(0.0f, 2.0f, -10.0f),
		Vector3(10.0f, 2.0f, 0.5f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 0.5f)));

	auto create_ball_cube = [&](const Vector3& offset, const Vector3& scale, float ballsize)
	{
		const int dims = 6;
		const Vector4 col = Vector4(1.0f, 0.5f, 0.2f, 1.0f);

		for (int x = 0; x < dims; ++x)
		{
			for (int y = 0; y < dims - 2; ++y)
			{
				for (int z = 0; z < dims; ++z)
				{
					Vector3 pos = offset + Vector3(scale.x *x, scale.y * y, scale.z * z);
					GameObject* sphere = BuildSphereObject(
						"",					// Optional: Name
						pos,				// Position
						ballsize / 2,			// Half-Dimensions
						true,				// Physics Enabled?
						10.f,				// Physical Mass (must have physics enabled)
						true,				// Physically Collidable (has collision shape)
						false,				// Dragable by user?
						col);// Render color
					this->AddGameObject(sphere);
				}
			}
		}
	};

	create_ball_cube(Vector3(0.0f, 0.5f, 0.f), Vector3(1.f, 1.f, 1.f), 1.f);
}


void TestScene::ShootABall()
{
	Vector3 velocity1 = GraphicsPipeline::Instance()->GetCamera()->GetDirection();
	GameObject* obj = CommonUtils::BuildSphereObject(
		"",
		GraphicsPipeline::Instance()->GetCamera()->GetPosition(),
		0.5f,
		true,
		1.0f,
		true,
		true,
		Vector4(1, 1, 1, 1));
	obj->Physics()->SetFriction(0.0f);
	obj->Physics()->SetElasticity(0.5f);
	obj->Physics()->SetLinearVelocity(velocity1*SpeedFactor);
	this->AddGameObject(obj);
}

void TestScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
}

void TestScene::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);
	NCLDebug::AddStatusEntry(Vector4(0.0f, 0.0f, 0.0f, 1.0f), "Score: %d", score);
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
	{
		//isScored = false;
		ShootABall();
	}
}
