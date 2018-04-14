/******************************************************************************
Class: Object
Implements:
Author:
Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:
This is the base class for all of the objects in your game/scenes. It
will automatically be managed by any Scene instance which it is added to,
firing OnRenderObject() and OnUpdateObject(float dt) each frame.

It can also optionally be attached to a PhysicsNode component which will
automatically update the object's world transform based off it's physical
position/orientation each frame.

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <nclgl\Matrix4.h>
#include <nclgl\RenderNode.h>
#include "GraphicsPipeline.h"
#include "PhysicsEngine.h"
#include "PhysicsNode.h"
#include <vector>
#include <functional>


class Scene;
class PhysicsEngine;

class GameObject
{
	friend class Scene;			//Can set the private variables scene
	friend class PhysicsEngine;	//Can notionally set private variable m_pPhysicsObject to NULL if it was deleted elsewhere

public:
	vector<PhysicsNode*>         PhysicsNodes;


	GameObject(const std::string& name = "")
		: friendlyName(name)
		, renderNode(NULL)
		, physicsNode(NULL)
	{
		type = 0;
	}

	GameObject(const std::string& name, RenderNode* renderNde, PhysicsNode* physicsNde = NULL, PhysicsNode* physicsNde2 = NULL)
		: friendlyName(name)
		, renderNode(renderNde)
		, physicsNode(physicsNde)
		, physicsNode2(physicsNde2)
	{
		if (physicsNde != NULL) {
			this->Physics()->SetParent(this);
		}

		if (physicsNde2 != NULL) {
			this->Physics2()->SetParent(this);
		}

		RegisterPhysicsToRenderTransformCallback();
		type = 0;
	}


	virtual ~GameObject()
	{
		if (renderNode)  GraphicsPipeline::Instance()->RemoveRenderNode(renderNode);
		if (physicsNode) PhysicsEngine::Instance()->RemovePhysicsObject(physicsNode);
		if (physicsNode2) PhysicsEngine::Instance()->RemovePhysicsObject(physicsNode2);

		SAFE_DELETE(renderNode);
		SAFE_DELETE(physicsNode);
		SAFE_DELETE(physicsNode2);
	}


	//<------- Scene Interation ------>
	// A good place to handle callback setups etc if you extend GameObject class (See ObjectPlayer for an example)
	virtual void OnAttachedToScene() {};
	virtual void OnDetachedFromScene() {};

	//<------- Object Parameters ------>
	inline const std::string& GetName() { return friendlyName; }
	inline const Scene* GetScene() const { return scene; }
	inline		 Scene* GetScene() { return scene; }


	//<---------- PHYSICS ------------>
	inline bool  HasPhysics() const { return (physicsNode != NULL); }
	inline bool	 HasPhycicesNodes() const { return (physicsNode != NULL); }
	inline bool  HasPhysics2() const { return (physicsNode2 != NULL); }


	inline const PhysicsNode*	Physics() const { return physicsNode; }
	inline		 PhysicsNode*	Physics() { return physicsNode; }
	inline const PhysicsNode*	Physics2() const { return physicsNode2; }
	inline		 PhysicsNode*	Physics2() { return physicsNode2; }

	vector<PhysicsNode*> GetPhysicsNodes()
	{
		return PhysicsNodes;
	}


	inline void  SetPhysics(PhysicsNode* node)
	{
		if (physicsNode)
		{
			UnregisterPhysicsToRenderTransformCallback(); //Unregister old callback listener
			physicsNode->SetParent(NULL);
		}

		physicsNode = node;

		if (physicsNode)
		{
			physicsNode->SetParent(this);
			RegisterPhysicsToRenderTransformCallback();   //Register new callback listener
		}
	}

	inline void  SetPhysics2(PhysicsNode* node)
	{
		if (physicsNode2)
		{
			UnregisterPhysics2ToRenderTransformCallback(); //Unregister old callback listener
			physicsNode2->SetParent(NULL);
		}

		physicsNode2 = node;

		if (physicsNode2)
		{
			physicsNode2->SetParent(this);
			RegisterPhysics2ToRenderTransformCallback();   //Register new callback listener
		}
	}

	inline void  SetPhysicsNodes(vector<PhysicsNode*> node)
	{
		if (PhysicsNodes.empty() == false)
		{
			UnregisterPhysicsNodesToRenderTransformCallback(); //Unregister old callback listener
			for (unsigned int i = 0; i < PhysicsNodes.size(); ++i) {
				PhysicsNodes[i]->SetParent(NULL);
			}
		}

		PhysicsNodes = node;

		if (PhysicsNodes.empty() == false)
		{
			for (int i = 0; i < PhysicsNodes.size(); ++i) {
				PhysicsNodes[i]->SetParent(this);
			}
			RegisterPhysicsNodesToRenderTransformCallback();   //Register new callback listener
		}
	}

	//<---------- GRAPHICS ------------>
	inline bool  HasRender() const { return (renderNode != NULL); }
	inline const RenderNode*	Render() const { return renderNode; }
	inline		 RenderNode*	Render() { return renderNode; }

	inline void  SetRender(RenderNode* node)
	{
		if (renderNode != node)
		{
			if (scene && renderNode) GraphicsPipeline::Instance()->RemoveRenderNode(node);

			renderNode = node;
			RegisterPhysicsToRenderTransformCallback();

			if (scene) GraphicsPipeline::Instance()->AddRenderNode(node);
		}
	}


	//---------- SOUND (?) ------------>




	//<---------- UTILS ------------>
	inline void RegisterPhysicsToRenderTransformCallback()
	{
		if (physicsNode && renderNode)
		{
			physicsNode->SetOnUpdateCallback(
				std::bind(
					&RenderNode::SetTransform,		// Function to call
					renderNode,					// Constant parameter (in this case, as a member function, we need a 'this' parameter to know which class it is)
					std::placeholders::_1)			// Variable parameter(s) that will be set by the callback function
			);
		}
	}

	inline void RegisterPhysics2ToRenderTransformCallback()
	{
		if (physicsNode2 && renderNode)
		{
			physicsNode2->SetOnUpdateCallback([&](const Matrix4& transform)
			{
				//physicsNode->worldTransform = renderNode->GetTransform();
			}
			);
			//physicsNode2->SetOnUpdateCallback(
			//	std::bind(
			//		&RenderNode::SetTransform,		// Function to call
			//		renderNode,					// Constant parameter (in this case, as a member function, we need a 'this' parameter to know which class it is)
			//		std::placeholders::_1)			// Variable parameter(s) that will be set by the callback function
			//);
		}
	}

	inline void RegisterPhysicsNodesToRenderTransformCallback()
	{
		if (PhysicsNodes.empty() == false && renderNode)
		{
			for (int i = 0; i < PhysicsNodes.size(); ++i) {
				PhysicsNodes[i]->SetOnUpdateCallback([&](const Matrix4& transform)
				{
					//PhysicsNodes[i]->worldTransform = renderNode->GetTransform();
				}
				);
			}
		}
	}

	inline void UnregisterPhysicsToRenderTransformCallback()
	{
		if (physicsNode)
		{
			physicsNode->SetOnUpdateCallback([](const Matrix4&) {});
		}
	}

	inline void UnregisterPhysics2ToRenderTransformCallback()
	{
		if (physicsNode2)
		{
			physicsNode2->SetOnUpdateCallback([](const Matrix4&) {});
		}
	}

	inline void UnregisterPhysicsNodesToRenderTransformCallback()
	{
		if (PhysicsNodes.empty() == false)
		{
			for (int i = 0; i < PhysicsNodes.size(); ++i) {
				PhysicsNodes[i]->SetOnUpdateCallback([](const Matrix4&) {});
			}
		}
	}

	void SetType(const int type) {
		this->type = type;
	}

	int GetType() { return type; }

protected:
	//Scene
	std::string					friendlyName;
	Scene*						scene;
	int type;

	//Components
	RenderNode*					renderNode;
	PhysicsNode*				physicsNode;
	PhysicsNode*                physicsNode2;

};
