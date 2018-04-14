
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\MazeGenerator.h>
#include <ncltech\MazeRenderer.h>
#include <nclgl\OBJMesh.h>
#include <ncltech\SearchAStar.h>
#include <nclgl\OBJMesh.h>
//Basic Network Example

class Net1_Client : public Scene
{
protected:
	GameObject* box;

	NetworkBase network;
	ENetPeer*	serverConnection;

	MazeGenerator* generator;
	MazeRenderer* maze;



	Mesh* wallmesh;

	SearchAStar* search_as;
	int astar_preset_idx;

	bool receivedInfo;

	float user_defined_size;

	coords2D userDefinedStart;
	coords2D userDefinedEnd;

	GameObject* playerAvater;
	GameObject* otherPlayers;
	GameObject* enemy;

	OBJMesh* dragon;


public:
	MazeGeneratorInfo3 data;
	Net1_Client(const std::string& friendly_name);

	float Info2[2];

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;
	//void MatchWithData(MazeGeneratorInfo* g1);
	void ProcessNetworkEvent(const ENetEvent& evnt);

	void DrawFinalPath();

	//Maze
	void GenerateNewMaze();

	//NewFunctions
	void CreatePlayerAvater();

	//New Variables
	vector<Vector3> finalPathCoords;
	vector<Vector3> finalPathExtendedCoords;
	bool avaterCreated;
	bool pathDrawn;
	bool UpdateAvaterPosition;
	bool spawnEnemy;
};

