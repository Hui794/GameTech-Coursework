#pragma once

#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\GameObject.h>
//#include "ObjectPlayer.h"
#include <nclgl\OBJMesh.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>

const float SpeedFactor = 30;

class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_MeshPlayer(new OBJMesh(MESHDIR"raptor.obj"))
	{
		ballID = 6;
		isScored = false;
		score = 0;

	}

	virtual ~EmptyScene() {}

	virtual void OnInitializeScene() override
	{
		//Set Defaults
		PhysicsEngine::Instance()->SetGravity(Vector3(0.0f, -9.81f, 0.0f));		//No Gravity!
		PhysicsEngine::Instance()->SetDampingFactor(1.0f);						//No Damping!

																				//Create Ground
		ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f), //center position
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)); //colour
		this->AddGameObject(ground);

		//sphere-cuboid case
		{
			Vector4 color1 = CommonUtils::GenColor(0.7f + 0.05f, 1.0f);
			GameObject* obj = CommonUtils::BuildCuboidObject(
				"",
				Vector3(10.0f, 0.5f, 10.0f),
				Vector3(0.5f, 0.5, 0.5),
				true,
				1.0f,
				true,
				true,
				color1);
			obj->Physics()->SetFriction(0.1f);
			obj->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj);

			GameObject* obj1 = CommonUtils::BuildSphereObject(
				"",
				Vector3(11.0f, 0.5f, 10.0f),
				0.5f,
				true,
				1.0f,
				true,
				true,
				color1);
			obj1->Physics()->SetFriction(0.0f);
			obj1->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj1);
		}


		//sphere-sphere case
		{
			Vector4 color1 = CommonUtils::GenColor(0.6f + 0.05f, 0.6f);
			GameObject* obj2 = CommonUtils::BuildSphereObject(
				"",
				Vector3(-10.0f, 0.5f, 10.0f),
				0.5f,
				true,
				1.0f,
				true,
				true,
				color1);
			obj2->Physics()->SetFriction(0.0f);
			obj2->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj2);

			GameObject* obj3 = CommonUtils::BuildSphereObject(
				"",
				Vector3(-11.0f, 0.5f, 10.0f),
				0.5f,
				true,
				1.0f,
				true,
				true,
				color1);
			obj3->Physics()->SetFriction(0.0f);
			obj3->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj3);
		}


		//cuboid-cuboid case
		{
			Vector4 color1 = CommonUtils::GenColor(0.6f + 0.05f, 0.6f);
			GameObject* obj4 = CommonUtils::BuildCuboidObject(
				"",
				Vector3(10.0f, 0.5f, -10.0f),
				Vector3(0.5f, 0.5, 0.5),
				true,
				1.0f,
				true,
				true,
				color1);
			obj4->Physics()->SetFriction(0.0f);
			obj4->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj4);

			GameObject* obj5 = CommonUtils::BuildCuboidObject(
				"",
				Vector3(10.0f, 0.5f, -11.0f),
				Vector3(0.5f, 0.5, 0.5),
				true,
				1.0f,
				true,
				true,
				color1);
			obj5->Physics()->SetFriction(0.0f);
			obj5->Physics()->SetElasticity(0.5f);
			this->AddGameObject(obj5);
		}


		{
			//Create Bouncing Spheres
			for (int i = 0; i <= 5; ++i)
			{
				Vector4 color = CommonUtils::GenColor(0.7f + i * 0.05f, 1.0f);
				GameObject* obj = CommonUtils::BuildSphereObject(
					"",
					Vector3(-6.0f + i * 1.25f, 5.5f, -5.0f),
					0.5f,
					true,
					1.0f,
					true,
					true,
					color);
				obj->Physics()->SetFriction(0.0f);
				obj->Physics()->SetElasticity(i * 0.2f);
				this->AddGameObject(obj);
			}
		}

		//Create Player
		m_Player = new GameObject("Player1");

		RenderNode* dummy = new RenderNode(m_MeshPlayer);
		dummy->SetTransform(Matrix4::Scale(Vector3(10, 10, 10))* Matrix4::Translation(Vector3(0, 0.48, 0)));
		m_Player->SetRender(new RenderNode());
		m_Player->Render()->AddChild(dummy);
		//player->Render()->SetTransform(Matrix4::Translation(Vector3(0,100,0)));


		//player->Render()->SetTransform(Matrix4::Scale(Vector3(10, 10, 10)));
		m_Player->Render()->SetBoundingRadius(1.0f);
		m_Player->Render()->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

		//Root Node
		m_Player->SetPhysics(new PhysicsNode());
		m_Player->Physics()->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
		m_Player->Physics()->SetCollisionShape(new CuboidCollisionShape(Vector3(0.01f, 0.01f, 0.01)));

		vector<PhysicsNode*> tmp_vec;

		//Feet
		PhysicsNode* temp = new PhysicsNode();
		temp->SetPosition(Vector3(-1.1f, 2.5f, 0.0f));
		temp->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 2.5f, 1.0f)));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(1.1f, 2.5f, 0.0f));
		temp->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 2.5f, 1.0f)));
		tmp_vec.push_back(temp);

		//Hands
		temp = new PhysicsNode();
		temp->SetPosition(Vector3(1.0f, 3.5f, -4.0f));
		temp->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 1.5f, 0.5f)));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(-1.0f, 3.5f, -4.0f));
		temp->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 1.5f, 0.5f)));
		tmp_vec.push_back(temp);

		//body
		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 6.0f, -1.55f));
		temp->SetCollisionShape(new CuboidCollisionShape(Vector3(1.25f, 1.0f, 3.0f)));
		tmp_vec.push_back(temp);

		//head
		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 9.0f, -5.75f));
		temp->SetCollisionShape(new SphereCollisionShape(1.0f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 8.5f, -7.5f));
		temp->SetCollisionShape(new SphereCollisionShape(0.75f));
		tmp_vec.push_back(temp);

		//neck
		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.5f, -5.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.65f));
		tmp_vec.push_back(temp);

		//tail collision shape
		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 6.0f, 2.5f));
		temp->SetCollisionShape(new SphereCollisionShape(1.0f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 6.5f, 4.5f));
		temp->SetCollisionShape(new SphereCollisionShape(1.0f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 6.75f, 6.5f));
		temp->SetCollisionShape(new SphereCollisionShape(1.0f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.0f, 8.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.1f, 9.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.2f, 10.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.3f, 11.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.4f, 12.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.5f, 13.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.6f, 14.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		temp = new PhysicsNode();
		temp->SetPosition(Vector3(0.0f, 7.7f, 15.0f));
		temp->SetCollisionShape(new SphereCollisionShape(0.5f));
		tmp_vec.push_back(temp);

		m_Player->SetPhysicsNodes(tmp_vec);


		this->AddGameObject(m_Player);
	}

	void ShootABall()
	{
		Vector3 velocity1 = GraphicsPipeline::Instance()->GetCamera()->GetDirection();
		GameObject* obj1 = CommonUtils::BuildSphereObject(
			"",
			GraphicsPipeline::Instance()->GetCamera()->GetPosition(),
			0.5f,
			true,
			1.0f,
			true,
			true,
			Vector4(1, 1, 1, 1));
		obj1->Physics()->SetFriction(0.0f);
		obj1->Physics()->SetElasticity(0.5f);
		obj1->Physics()->SetLinearVelocity(velocity1*SpeedFactor);
		obj1->Physics()->SetOnCollisionCallback(
			std::bind(&EmptyScene::ExampleCallbackFunction2,
				this,							//Any non-placeholder param will be passed into the function each time it is called
				std::placeholders::_1,			//The placeholders correlate to the expected parameters being passed to the callback
				std::placeholders::_2
			));
		this->AddGameObject(obj1);
	}

	virtual void OnUpdateScene(float dt) override
	{
		const float mv_speed = 5.f * dt;			//Meters per second
		const float rot_speed = 90.f * dt;			//Rotational degrees per second
		Scene::OnUpdateScene(dt);
		NCLDebug::AddStatusEntry(Vector4(0.0f, 0.0f, 0.0f, 1.0f), "Score: %d", score);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		{
			isScored = false;
			ShootABall();
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
		{
			m_Player->Physics()->SetPosition(m_Player->Physics()->GetPosition() +
				m_Player->Physics()->GetOrientation().ToMatrix3() * Vector3(0.0f, 0.0f, -mv_speed));
			for (int i = 0; i < m_Player->PhysicsNodes.size(); ++i) {
				m_Player->PhysicsNodes[i]->SetPosition(m_Player->PhysicsNodes[i]->GetPosition() + m_Player->PhysicsNodes[i]->GetOrientation().ToMatrix3() * Vector3(0.0f, 0.0f, -mv_speed));
			}
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		{
			m_Player->Physics()->SetPosition(m_Player->Physics()->GetPosition() +
				m_Player->Physics()->GetOrientation().ToMatrix3()* Vector3(0.0f, 0.0f, mv_speed));
			for (int i = 0; i < m_Player->PhysicsNodes.size(); ++i) {
				m_Player->PhysicsNodes[i]->SetPosition(m_Player->PhysicsNodes[i]->GetPosition() + m_Player->PhysicsNodes[i]->GetOrientation().ToMatrix3() * Vector3(0.0f, 0.0f, mv_speed));
			}
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		{
			/*Physics()->SetOrientation(Physics()->GetOrientation() *
			Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), rot_speed));*/

			m_Player->Physics()->SetPosition(m_Player->Physics()->GetPosition() +
				m_Player->Physics()->GetOrientation().ToMatrix3() * Vector3(-mv_speed, 0.0f, 0.0f));

			for (int i = 0; i < m_Player->PhysicsNodes.size(); ++i) {
				m_Player->PhysicsNodes[i]->SetPosition(m_Player->PhysicsNodes[i]->GetPosition() + m_Player->PhysicsNodes[i]->GetOrientation().ToMatrix3() * Vector3(-mv_speed, 0.0f, 0.0f));
				/*PhysicsNodes[i]->SetOrientation(PhysicsNodes[i]->GetOrientation() *
				Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), rot_speed));*/
			}
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		{
			/*Physics()->SetOrientation(Physics()->GetOrientation() *
			Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), -rot_speed));*/

			m_Player->Physics()->SetPosition(m_Player->Physics()->GetPosition() +
				m_Player->Physics()->GetOrientation().ToMatrix3() * Vector3(mv_speed, 0.0f, 0.0f));

			for (int i = 0; i < m_Player->PhysicsNodes.size(); ++i) {
				m_Player->PhysicsNodes[i]->SetPosition(m_Player->PhysicsNodes[i]->GetPosition() + m_Player->PhysicsNodes[i]->GetOrientation().ToMatrix3() * Vector3(mv_speed, 0.0f, 0.0f));
				/*PhysicsNodes[i]->SetOrientation(PhysicsNodes[i]->GetOrientation() *
				Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), -rot_speed));*/
			}
		}
	}

	bool ExampleCallbackFunction2(PhysicsNode* self, PhysicsNode* collidingObject)
	{
		if (isScored == false && collidingObject->GetParent()->GetType() == 1) {
			score += 100;
			cout << score << " ";
			isScored = true;
		}
		else if (isScored == false && collidingObject->GetParent()->GetType() == 2) {
			score -= 50;
			cout << score << " ";
			isScored = true;
		}
		//Return true to enable collision resolution, for AI test's just return false so we can drop the collision pair from the system
		return true;
	}

protected:
	int ballID;
	int ballDistance;
	//°Ð×Ó
	Mesh*					m_TargetMesh;
	//Çò
	GameObject*				m_Sphere;
	//Å×ÎïÏß
	std::vector<Vector3>	m_TrajectoryPoints;
	bool isScored;
	int score;

	GameObject* ground;

	GameObject* m_Player;

	OBJMesh* m_MeshPlayer;
};