
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
1. right click on the entire solution (top of the solution exporerer) and go to properties
2. Go to Common Properties -> Statup Project
3. Select Multiple Statup Projects
4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



(\_/)
( '_')
/""""""""""""\=========     -----D
/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\MazeGenerator.h>
#include <ncltech\SearchAStar.h>

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")



#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second


NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
float rotation = 0.0f;

void Win32_PrintAllAdapterIPAddresses();

class MazeServer
{
protected:

	SearchAStar* search_as;

	coords2D userDefinedStart;
	coords2D userDefinedEnd;



	float scaleFactor = 6;
	const Vector3 pos_maze1 = Vector3(0.f, 0.f, 0.f);
	float detailLevel = 20;
public:
	bool undateAvaterPosition = false;
	bool updateEnemyPosition = false;
	MazeGenerator* generator;
	vector<Vector3> finalPathCoords;
	vector<Vector3> finalPathExtendedCoords;
	vector<Vector3> enemyPathCoords;
	vector<Vector3> finalPathExtendedCoords2;
	int clientNum;

	MazeServer()
	{
		generator = new MazeGenerator();
		search_as = new SearchAStar();
		userDefinedStart.x = 0;
		userDefinedStart.y = 0;
		userDefinedEnd.x = 0;
		userDefinedEnd.y = 0;
		clientNum = 0;
	}

	void SearchEnemyPath(GraphNode* start1, GraphNode* end1)
	{
		float weightingG = 1.0f;
		float weightingH = 1.0f;
		search_as->SetWeightings(weightingG, weightingH);
		search_as->FindBestPath(start1, end1);

		std::list<const GraphNode*>::iterator temp;
		temp = search_as->finalPath.begin();
		Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f * scaleFactor, 5.0f / float(generator->GetSize()) * scaleFactor, 5.f * scaleFactor)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));
		Matrix4 transform = Matrix4::Translation(pos_maze1) * maze_scalar;

		float grid_scalar = 1.0f / (float)generator->GetSize();
		
		for (; temp != search_as->finalPath.end(); temp++)
		{
			Vector3 tmp_vec3;
			tmp_vec3.y = 0.2f;
			tmp_vec3.x = ((*temp)->_pos.x + 0.6) * grid_scalar;
			tmp_vec3.z = ((*temp)->_pos.y + 0.6) * grid_scalar;
			tmp_vec3 = transform * tmp_vec3;
			enemyPathCoords.push_back(tmp_vec3);
		}
	}

	void SearchUsingAStar()
	{
		finalPathCoords.clear();
		float weightingG, weightingH;
		weightingG = 1.0f;
		weightingH = 1.0f;
		search_as->SetWeightings(weightingG, weightingH);
		GraphNode* start = generator->GetStartNode();
		GraphNode* end = generator->GetGoalNode();
		search_as->FindBestPath(start, end);

		std::list<const GraphNode*>::iterator temp;
		temp = search_as->finalPath.begin();
		Matrix4 maze_scalar = Matrix4::Scale(Vector3(5.f * scaleFactor, 5.0f / float(generator->GetSize()) * scaleFactor, 5.f * scaleFactor)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));
		Matrix4 transform = Matrix4::Translation(pos_maze1) * maze_scalar;

		float grid_scalar = 1.0f / (float)generator->GetSize();

		for (; temp != search_as->finalPath.end(); temp++)
		{
			Vector3 tmp_vec3;
			tmp_vec3.y = 0.2f;
			tmp_vec3.x = ((*temp)->_pos.x + 0.6) * grid_scalar;
			tmp_vec3.z = ((*temp)->_pos.y + 0.6) * grid_scalar;
			tmp_vec3 = transform * tmp_vec3;
			finalPathCoords.push_back(tmp_vec3);
		}
	}

	void ExtendFinalPathCoords()
	{
		finalPathExtendedCoords.clear();
		for (int i = 0; i < finalPathCoords.size() - 1; ++i) {
			Vector3 left = finalPathCoords[i];
			Vector3 right = finalPathCoords[i + 1];
			float offset_x = (right.x - left.x) / detailLevel;
			float offset_z = (right.z - left.z) / detailLevel;
			for (int j = 0; j < detailLevel; ++j) {
				Vector3 temp = left;
				temp.x += offset_x * j;
				temp.z += offset_z * j;
				finalPathExtendedCoords.push_back(temp);
			}
		}
	}


};

int onExit(int exitcode)
{
	server.Release();
	//system("pause");
	exit(exitcode);
}



int main(int arcg, char** argv)
{
	MazeGeneratorInfo3* maze_Generator3;
	maze_Generator3 = new MazeGeneratorInfo3();
	MazeGeneratorInfo3* maze_Generator4;
	maze_Generator4 = new MazeGeneratorInfo3();
	MazeServer _Maze;
	MazeServer _Client2;
	//generator = new MazeGenerator();
	
	//test1 = new test();
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");


	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("- New Client Connected\n");
				break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				if (evnt.packet->dataLength == sizeof(float) * 6)
				{

					//printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
					printf("\t Client %d says: ", evnt.peer->incomingPeerID);
					float* temp = NULL;

					//memcpy(temp, evnt.packet->data, sizeof(int)*2);
					temp = (float*)evnt.packet->data;
					std::cout << "Size of the maze is " << temp[0] << " " << "and density for the maze is " << temp[1] << std::endl;

					std::cout << "The 2D Coords of client defined Start Point is: x = " << temp[2] << " y = " << temp[3] << endl;
					std::cout << "The 2D Coords of client defined End Point is: x = " << temp[4] << " y = " << temp[5] << endl;

					if (evnt.peer->incomingPeerID == 0) {
						_Maze.generator->Generate(temp[0], (float)temp[1]);
						_Maze.generator->GetUserDefineStartEnd(temp[2], temp[3], temp[4], temp[5]);
						enet_packet_destroy(evnt.packet);

						_Maze.SearchUsingAStar();
						_Maze.ExtendFinalPathCoords();

						maze_Generator3->client_id = 0;
						_Maze.generator->CollectGeneratorInfo(*maze_Generator3);
						maze_Generator3->sizeA = _Maze.finalPathCoords.size();
						maze_Generator3->sizeB = _Maze.finalPathExtendedCoords.size();

						for (int i = 0; i < _Maze.finalPathCoords.size(); ++i) {
							maze_Generator3->finalPathCoords[i] = _Maze.finalPathCoords[i];
						}

						for (int i = 0; i < _Maze.finalPathExtendedCoords.size(); ++i) {
							maze_Generator3->ExtendedfinalPathCoords[i] = _Maze.finalPathExtendedCoords[i];
						}

						ENetPacket* position_update = enet_packet_create(maze_Generator3, sizeof(MazeGeneratorInfo3), 0);
						enet_host_broadcast(server.m_pNetwork, 0, position_update);
					}
					else if (evnt.peer->incomingPeerID == 1)
					{
						_Client2.generator->Generate(temp[0], (float)temp[1]);
						_Client2.generator->GetUserDefineStartEnd(temp[2], temp[3], temp[4], temp[5]);
						enet_packet_destroy(evnt.packet);

						_Client2.SearchUsingAStar();
						_Client2.ExtendFinalPathCoords();

						maze_Generator4->client_id = 1;
						_Client2.generator->CollectGeneratorInfo(*maze_Generator4);
						maze_Generator4->sizeA = _Client2.finalPathCoords.size();
						maze_Generator4->sizeB = _Client2.finalPathExtendedCoords.size();

						for (int i = 0; i < _Client2.finalPathCoords.size(); ++i) {
							maze_Generator4->finalPathCoords[i] = _Client2.finalPathCoords[i];
						}

						for (int i = 0; i < _Client2.finalPathExtendedCoords.size(); ++i) {
							maze_Generator4->ExtendedfinalPathCoords[i] = _Client2.finalPathExtendedCoords[i];
						}

						ENetPacket* position_update = enet_packet_create(maze_Generator4, sizeof(MazeGeneratorInfo3), 0);
						enet_host_broadcast(server.m_pNetwork, 0, position_update);
					}
				}
				else if (evnt.packet->dataLength == sizeof(bool))
				{
					if (evnt.peer->incomingPeerID == 0)
					{
						_Maze.undateAvaterPosition = true;
					}
					else {
						_Client2.undateAvaterPosition = true;
					}
				}
				else if (evnt.packet->dataLength == sizeof(int))
				{
					int *temp = (int*)evnt.packet->data;
					if (*temp == 3)
					{
						int a1;
						if (evnt.peer->incomingPeerID == 0)
						{
							a1 = 0;
							ENetPacket* client_id = enet_packet_create(&a1, sizeof(int), 0);
							enet_host_broadcast(server.m_pNetwork, 0, client_id);
						}
						else if (evnt.peer->incomingPeerID == 1)
						{
							a1 = 1;
							ENetPacket* client_id = enet_packet_create(&a1, sizeof(int), 0);
							enet_host_broadcast(server.m_pNetwork, 0, client_id);
						}
					}
					if (evnt.peer->incomingPeerID == 0)
					{

						if (*temp == 1) {
							cout << "Server will now spawn an enemy to chase player" << endl;
							_Maze.updateEnemyPosition = true;
							int x = rand() % 15;
							int y = rand() % 15;
							_Maze.enemyPathCoords.clear();
							_Maze.SearchEnemyPath(&(_Maze.generator->allNodes[x * _Maze.generator->size + y]), _Maze.generator->end);
						}
						else if (*temp == 2) {
							_Maze.generator->Generate(16, 70);
							_Maze.generator->GetUserDefineStartEnd(0, 15, 15, 0);
							enet_packet_destroy(evnt.packet);

							_Maze.SearchUsingAStar();
							_Maze.ExtendFinalPathCoords();

							_Maze.generator->CollectGeneratorInfo(*maze_Generator3);
							maze_Generator3->sizeA = _Maze.finalPathCoords.size();
							maze_Generator3->sizeB = _Maze.finalPathExtendedCoords.size();

							for (int i = 0; i < _Maze.finalPathCoords.size(); ++i) {
								maze_Generator3->finalPathCoords[i] = _Maze.finalPathCoords[i];
							}

							for (int i = 0; i < _Maze.finalPathExtendedCoords.size(); ++i) {
								maze_Generator3->ExtendedfinalPathCoords[i] = _Maze.finalPathExtendedCoords[i];
							}

							ENetPacket* position_update = enet_packet_create(maze_Generator3, sizeof(MazeGeneratorInfo3), 0);
							enet_host_broadcast(server.m_pNetwork, 0, position_update);
						}
					}
					else if (evnt.peer->incomingPeerID == 1)
					{
						int *temp = (int*)evnt.packet->data;
						if (*temp == 1) {
							cout << "Server will now spawn an enemy to chase player" << endl;
							_Client2.updateEnemyPosition = true;
							int x = rand() % 15;
							int y = rand() % 15;
							_Client2.enemyPathCoords.clear();
							_Client2.SearchEnemyPath(&(_Client2.generator->allNodes[x * _Client2.generator->size + y]), _Client2.generator->end);
						}
						else if (*temp == 2) {
							_Client2.generator->Generate(16, 70);
							_Client2.generator->GetUserDefineStartEnd(0, 15, 15, 0);
							enet_packet_destroy(evnt.packet);

							_Client2.SearchUsingAStar();
							_Client2.ExtendFinalPathCoords();

							_Client2.generator->CollectGeneratorInfo(*maze_Generator3);
							maze_Generator3->sizeA = _Client2.finalPathCoords.size();
							maze_Generator3->sizeB = _Client2.finalPathExtendedCoords.size();

							for (int i = 0; i < _Client2.finalPathCoords.size(); ++i) {
								maze_Generator3->finalPathCoords[i] = _Client2.finalPathCoords[i];
							}

							for (int i = 0; i < _Client2.finalPathExtendedCoords.size(); ++i) {
								maze_Generator3->ExtendedfinalPathCoords[i] = _Client2.finalPathExtendedCoords[i];
							}

							ENetPacket* position_update = enet_packet_create(maze_Generator3, sizeof(MazeGeneratorInfo3), 0);
							enet_host_broadcast(server.m_pNetwork, 0, position_update);
						}
					}
				}
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		});

		static int index1 = 0;
		static int index2 = 0;
		static int index3 = 0;
		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP)
		{
			accum_time = 0.0f;
			int length = _Maze.finalPathExtendedCoords.size();
			if (_Maze.undateAvaterPosition == true) {
				if (index1 < length - 1) {
					//Vector3 pos = _Maze.finalPathExtendedCoords[index];
					PositionInfo pos;
					pos.id = 1;
					pos.position = _Maze.finalPathExtendedCoords[index1];
					ENetPacket* position_update = enet_packet_create(&pos, sizeof(PositionInfo), 0);
					enet_host_broadcast(server.m_pNetwork, 0, position_update);
					index1++;
				}
				else {
					index1 = 0;
				}
			}

			int length2 = _Client2.finalPathExtendedCoords.size();
			if (_Client2.undateAvaterPosition == true)
			{
				if (index2 < length2 - 1) {
					//Vector3 pos = _Maze.finalPathExtendedCoords[index];
					PositionInfo pos;
					pos.id = 3;
					pos.position =_Client2.finalPathExtendedCoords[index2];
					ENetPacket* position_update = enet_packet_create(&pos, sizeof(PositionInfo), 0);
					enet_host_broadcast(server.m_pNetwork, 0, position_update);
					index2++;
				}
				else {
					index2 = 0;
				}
			}

			int length1 = _Maze.enemyPathCoords.size();
			static int flag1 = 20;
			if (flag1 == 20) {
				if (_Maze.updateEnemyPosition == true)
				{
					flag1 = 0;
					if (index2 < length1 - 1) {
						//Vector3 pos = _Maze.finalPathExtendedCoords[index];
						PositionInfo pos;
						pos.id = 2;
						pos.position = _Maze.enemyPathCoords[index2];
						ENetPacket* position_update = enet_packet_create(&pos, sizeof(PositionInfo), 0);
						enet_host_broadcast(server.m_pNetwork, 0, position_update);
						index2++;
					}
					else {
						index2 = 0;
					}
				}
			}
			else {
				flag1++;
			}


			//Packet data
			// - At the moment this is just a position update that rotates around the origin of the world
			//   though this can be any variable, structure or class you wish. Just remember that everything 
			//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)

			//Vector3 pos = Vector3(
			//	cos(rotation) * 2.0f,
			//	1.5f,
			//	sin(rotation) * 2.0f);

			//pos = _Maze.finalPathExtendedCoords[index];


			//Create the packet and broadcast it (unreliable transport) to all clients
			//ENetPacket* position_update = enet_packet_create(&pos, sizeof(Vector3), 0);

		}

		Sleep(0);
	}

	//system("pause");
	server.Release();
}




//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;


	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}


	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}

}