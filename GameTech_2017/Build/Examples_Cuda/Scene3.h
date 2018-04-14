#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\NCLDebug.h>

const float Num = 25;
const float Start_x = -2;
const float Start_y = 8;
const float Start_z = 0;
const float offset_x = 1;
const float offset_y = 1;

class SoftBody : public Scene
{
public:
	SoftBody(const std::string& friendly_name)
		: Scene(friendly_name)
	{
		row = sqrt(Num);
		col = row;
		softBody = new GameObject*[Num];
	}

	//-4£¬ 6 £¬0
	void CreateASoftBody()
	{
		DistanceConstraint* constraint;
		for (int i = 0; i < row; ++i) {
			for (int j = 0; j < col; ++j) {
				Vector3 position(Start_x + offset_x*j, Start_y - offset_y * i, Start_z);
				if (i == 0) {
					softBody[i*row + j] = CommonUtils::BuildSphereObject("",
						position,				//Position
						0.2f,									//Radius
						true,									//Has Physics Object
						0.0f,									//Infinite Mass
						true,									//No Collision Shape Yet
						true,									//Dragable by the user
						CommonUtils::GenColor(0.45f, 1.0f));	//Color
					this->AddGameObject(softBody[i*row + j]);
				}
				else {
					softBody[i*row + j] = CommonUtils::BuildSphereObject("",
						position,				//Position
						0.2f,									//Radius
						true,									//Has Physics Object
						1.0f,									//Infinite Mass
						true,									//No Collision Shape Yet
						true,									//Dragable by the user
						CommonUtils::GenColor(0.45f, 1.0f));	//Color
					this->AddGameObject(softBody[i*row + j]);

					PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
						softBody[i*row + j]->Physics(),					//Physics Object A
						softBody[(i - 1)*row + j]->Physics(),					//Physics Object B
						softBody[i*row + j]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						softBody[(i - 1)*row + j]->Physics()->GetPosition()));
				}


				if (j > 0) {
					PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
						softBody[i*row + j]->Physics(),					//Physics Object A
						softBody[i*row + j - 1]->Physics(),					//Physics Object B
						softBody[i*row + j]->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
						softBody[i*row + j - 1]->Physics()->GetPosition()));	//Attachment Position on Object B	-> Currently the centre  	//Attachment Position on Object B	-> Currently the far left edge 
				}


			}
		}
	}

	virtual void OnInitializeScene() override
	{
		PhysicsEngine::Instance()->SetGravity(Vector3(0.0f, -9.8f, 0.0f));		//No Gravity!
		PhysicsEngine::Instance()->SetDampingFactor(0.99f);						//No Damping!

																				//Create Ground (..why not?)
		GameObject* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));

		this->AddGameObject(ground);

		GameObject *handle, *ball;

		CreateASoftBody();

		//CreateAtail();

		////Create Hanging Cube (Attached by corner)
		//handle = CommonUtils::BuildSphereObject("",
		//	Vector3(4.f, 7.f, -5.0f),				//Position
		//	1.0f,									//Radius
		//	true,									//Has Physics Object
		//	0.1f,									//Inverse Mass = 1 / 10 kg mass (For creating rotational inertia tensor)
		//	false,									//No Collision Shape Yet
		//	true,									//Dragable by the user
		//	CommonUtils::GenColor(0.55f, 0.5f));	//Color

		//											//Set linear mass to be infinite, so it can rotate still but not move
		//handle->Physics()->SetInverseMass(0.0f);

		//ball = CommonUtils::BuildCuboidObject("",
		//	Vector3(7.f, 7.f, -5.0f),				//Position
		//	Vector3(0.5f, 0.5f, 0.5f),				//Half Dimensions
		//	true,									//Has Physics Object
		//	1.0f,									//Inverse Mass = 1 / 1kg mass
		//	false,									//No Collision Shape Yet
		//	true,									//Dragable by the user
		//	CommonUtils::GenColor(0.6f, 1.0f));		//Color

		//this->AddGameObject(handle);
		//this->AddGameObject(ball);

		//PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
		//	handle->Physics(),													//Physics Object A
		//	ball->Physics(),													//Physics Object B
		//	handle->Physics()->GetPosition() + Vector3(1.0f, 0.0f, 0.0f),		//Attachment Position on Object A	-> Currently the far right edge
		//	ball->Physics()->GetPosition() + Vector3(-0.5f, -0.5f, -0.5f)));	//Attachment Position on Object B	-> Currently the far left edge 

	}

	void CreateAtail()
	{
		GameObject *handle, *ball;
		//Create Hanging Ball
		handle = CommonUtils::BuildSphereObject("",
			Vector3(-7.f, 7.f, -5.0f),				//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			0.0f,									//Infinite Mass
			true,									//No Collision Shape Yet
			true,									//Dragable by the user
			CommonUtils::GenColor(0.45f, 0.5f));	//Color

		ball = CommonUtils::BuildSphereObject("",
			Vector3(-4.f, 7.f, -5.0f),				//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f,									// Inverse Mass = 1 / 1kg mass
			true,									//No Collision Shape Yet
			true,									//Dragable by the user
			CommonUtils::GenColor(0.5f, 1.0f));		//Color

		this->AddGameObject(handle);
		this->AddGameObject(ball);

		GameObject * ball1 = CommonUtils::BuildSphereObject("",
			Vector3(-3.f, 6.f, -3.0f),				//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f,									// Inverse Mass = 1 / 1kg mass
			true,									//No Collision Shape Yet
			true,									//Dragable by the user
			CommonUtils::GenColor(0.5f, 1.0f));		//Color
		this->AddGameObject(ball1);


		//Add distance constraint between the two objects
		DistanceConstraint* constraint = new DistanceConstraint(
			handle->Physics(),					//Physics Object A
			ball->Physics(),					//Physics Object B
			handle->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
			ball->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
		PhysicsEngine::Instance()->AddConstraint(constraint);

		PhysicsEngine::Instance()->AddConstraint(new DistanceConstraint(
			ball->Physics(),					//Physics Object A
			ball1->Physics(),					//Physics Object B
			ball->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
			ball1->Physics()->GetPosition()));	//Attachment Position on Object B	-> Currently the centre  	//Attachment Position on Object B	-> Currently the far left edge 
	}
protected:
	int row;
	int col;
	GameObject** softBody;
};