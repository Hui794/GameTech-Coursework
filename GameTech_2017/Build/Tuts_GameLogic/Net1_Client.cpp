/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
1. right click on the entire solution (top of the solution exporerer) and go to properties
2. Go to Common Properties -> Statup Project
3. Select Multiple Statup Projects
4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!"
if it successfully connects. The server, will continually broadcast a packet containing a
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
1. Try setting up both the server and client within the same Scene (disabling collisions
on the objects as they now share the same physics engine). This way we can clearly
see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
which is found within the root directory of this framework that allows you to inject
latency/packet loss etc on network. Try messing around with various latency/packet-loss
values.

2. Packet Loss
This causes the object to jump in large (and VERY noticable) gaps from one position to
another.

A good place to start in compensating for this is to build a buffer and store the
last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
that by the time we need that position update, we will already have the next position
packet which we can use to interpolate that missing data from. The number of packets we
will need to backup will be relative to the amount of expected packet loss. This method
will also insert additional 'buffer' latency to our system, so we better not make it wait
too long.

3. Latency
There is no easy way of solving this, and will have all felt it's punishing effects
on all networked games. The way most games attempt to hide any latency is by actually
running different games on different machines, these will react instantly to your actions
such as shooting which the server will eventually process at some point and tell you if you
have hit anything. This makes it appear (client side) like have no latency at all as you
moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
and mirrors and the server is still doing all the hard stuff (except now it has to take into account
the fact that you shot at time - latency time).

This smoke and mirrors approach also leads into another major issue, which is what happens when
the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
hit. This leads to issues which the server has to decipher on it's own, this will also happen
alot with generic physical elements which will ocasional 'snap' back to it's actual position on
the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>


//#include <Tuts_Network_Server\MazeGenerator.h>
const Vector3 pos_maze1 = Vector3(0.f, 0.f, 0.f);
const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

const float scaleFactor = 6;

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, box(NULL)
	, astar_preset_idx(2)
	, search_as(new SearchAStar())
	, receivedInfo(false)
	, user_defined_size(0)
	, dragon(new OBJMesh(MESHDIR"raptor.obj"))
{
	generator = new MazeGenerator();

	wallmesh = new OBJMesh(MESHDIR"cube.obj");

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallmesh->SetTexture(whitetex);

	srand(93225);

	userDefinedStart.x = 0;
	userDefinedStart.y = 0;
	userDefinedEnd.x = 0;
	userDefinedEnd.y = 0;

	avaterCreated = false;
	pathDrawn = false;
}

void Net1_Client::OnInitializeScene()
{
	//Initialize Client Network


	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}

	//Generate Simple Scene with a box that can be updated upon recieving server packets
	box = CommonUtils::BuildCuboidObject(
		"Server",
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.5f, 0.5f, 0.5f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));
	//this->AddGameObject(box);

	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(-1.5, 25, 1));
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(-80);
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(0);

	//Create Ground (..we still have some common ground to work off)


	//GenerateNewMaze();
}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	box = NULL; // Deleted in above function

	SAFE_DELETE(wallmesh);

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	static int flag = 0;

	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);



	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
		"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);


	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);

	if (receivedInfo == true) {
		maze->DrawSearchHistory(search_as->GetSearchHistory(), 2.5f / float(generator->GetSize()) * (scaleFactor - 1));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F1))
	{
		if (avaterCreated == false) {
			CreatePlayerAvater();
			avaterCreated = true;
		}

	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F2))
	{
		if (pathDrawn == false) {
			DrawFinalPath();
			pathDrawn = true;
		}
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_F3))
	{
		UpdateAvaterPosition = true;

		ENetPacket* temp = enet_packet_create(&UpdateAvaterPosition, sizeof(bool), 0);

		enet_peer_send(serverConnection, 0, temp);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_F4))
	{
		flag = 1;
		spawnEnemy = true;
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_F5))
	{
		int a = 2;
		ENetPacket* temp = enet_packet_create(&a, sizeof(int), 0);
		enet_peer_send(serverConnection, 0, temp);
	}

	if (flag == 1) {
		int a = 1;
		ENetPacket* temp = enet_packet_create(&a, sizeof(int), 0);
		enet_peer_send(serverConnection, 0, temp);
		enemy = CommonUtils::BuildSphereObject(
			"",
			finalPathCoords[1],
			0.8f,
			true,
			0.0f,
			false,
			false,
			Vector4(0, 1, 0, 1));
		this->AddGameObject(enemy);
		flag = 0;
	}

}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == serverConnection)
		{
			NCLDebug::Log(status_color3, "Network: Successfully connected to server!\n");
			NCLDebug::Log(status_color3, "Please provide the size of the maze for the server to generate!\n");

			cin >> user_defined_size;
			while (user_defined_size <= 0 || user_defined_size >= 50) {
				NCLDebug::Log(status_color3, "Size must be integers less than 50 and greater than 0\n");
				cin >> user_defined_size;
			}

			NCLDebug::Log(status_color3, "Please type in the start and end coords of the maze for the server to calculate a route!\n");

			for (int i = 0; i < 10; ++i) {
				cin >> userDefinedStart.x >> userDefinedStart.y;
				cin >> userDefinedEnd.x >> userDefinedEnd.y;
				bool a = userDefinedStart.x >= 0;
				bool b = userDefinedStart.x <= 15;
				bool c = userDefinedStart.y >= 0;
				bool d = userDefinedStart.y <= 15;
				bool e = userDefinedEnd.x >= 0;
				bool f = userDefinedEnd.x <= 15;
				bool g = userDefinedEnd.y >= 0;
				bool h = userDefinedEnd.y <= 15;
				if (a + b + c + d + e + f + g + h == 8) {
					break;
				}
			}

			float info2[6] = { user_defined_size, 50, userDefinedStart.x, userDefinedStart.y, userDefinedEnd.x, userDefinedEnd.y };

			//float info2[6] = { 16, 50, 13, 0, 0, 15 };

			/*userDefinedStart.x = 13;
			userDefinedStart.y = 0;
			userDefinedEnd.x = 0;
			userDefinedEnd.y = 15;*/

			ENetPacket* packet = enet_packet_create(info2, sizeof(info2), 0);

			enet_peer_send(serverConnection, 0, packet);
		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{
		if (evnt.packet->dataLength == sizeof(MazeGeneratorInfo3)) {
			MazeGeneratorInfo3* temp = (MazeGeneratorInfo3*)(evnt.packet->data);

			if (temp->client_id == 1) {
				generator->data = (*temp);

				generator->density1 = 1.0f;

				generator->GetMazeInfo();

				finalPathCoords.clear();
				finalPathExtendedCoords.clear();

				for (int i = 0; i < temp->sizeA; ++i) {
					finalPathCoords.push_back(temp->finalPathCoords[i]);
				}

				for (int i = 0; i < temp->sizeB; ++i) {
					finalPathExtendedCoords.push_back(temp->ExtendedfinalPathCoords[i]);
				}

				GenerateNewMaze();
				receivedInfo = true;
				enet_packet_destroy(evnt.packet);
			}
		}
		else if (evnt.packet->dataLength == sizeof(PositionInfo))
		{
			//cout << "hahahahah" << endl;
			PositionInfo* pos = (PositionInfo*)evnt.packet->data;
			Vector3 temp;
			memcpy(&temp, &(pos->position), sizeof(Vector3));
			if (pos->id == 1) {
				if (UpdateAvaterPosition == true) {
					playerAvater->Physics()->SetPosition(temp);
				}
			}
			if (pos->id == 2) {
				if (spawnEnemy == true) {
					enemy->Physics()->SetPosition(temp);
				}
			}

			//
			//box->Physics()->SetPosition(pos);
			enet_packet_destroy(evnt.packet);
		}
		else
		{
			NCLERROR("Recieved Invalid Network Packet!");
		}
	}
	break;
	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		NCLDebug::Log(status_color3, "Network: Server has disconnected!");
	}
	break;
	}
}

void Net1_Client::GenerateNewMaze()
{
	this->DeleteAllGameObjects(); //Cleanup old mazes
								  //generator->Generate(16, 100);
								  //generator->GenerateAnditionalData();
								  //generator->GetUserDefineStartEnd(userDefinedStart.x, userDefinedStart.y, userDefinedEnd.x, userDefinedEnd.y);

	GameObject* ground = CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		false,
		0.0f,
		false,
		false,
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));

	this->AddGameObject(ground);

	//The maze is returned in a [0,0,0] - [1,1,1] cube (with edge walls outside) regardless of grid_size,
	// so we need to scale it to whatever size we want
	Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f * scaleFactor, 5.0f / float(generator->size) * scaleFactor, 5.f * scaleFactor)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));

	maze = new MazeRenderer(generator, wallmesh);
	maze->Render()->SetTransform(Matrix4::Translation(pos_maze1) * maze_scalar);
	this->AddGameObject(maze);
	GraphNode* start = generator->GetStartNode();
	GraphNode* end = generator->GetGoalNode();
}

void Net1_Client::CreatePlayerAvater()
{

	avaterCreated = true;

	playerAvater = CommonUtils::BuildSphereObject(
		"",
		finalPathCoords[1],
		0.8f,
		true,
		0.0f,
		false,
		false,
		Vector4(1, 0, 0, 1));
	this->AddGameObject(playerAvater);

}

void Net1_Client::DrawFinalPath()
{
	for (int i = 0; i < finalPathCoords.size(); ++i) {
		GameObject* obj = CommonUtils::BuildCuboidObject(
			"",
			finalPathCoords[i],
			Vector3(0.5f, 0.5, 0.5),
			false,
			0.5f,
			false,
			false,
			Vector4(1, 1, 1, 0.05));
		this->AddGameObject(obj);
	}
}