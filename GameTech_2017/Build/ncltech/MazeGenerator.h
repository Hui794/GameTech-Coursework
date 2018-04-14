#pragma once
#include <ncltech\GameObject.h>
#include <ncltech\Scene.h>
#include "SearchAlgorithm.h"


struct coords2D
{
	float x;
	float y;
};

struct MazeEdge
{
	float weighting;
	bool _connected;
	bool _iswall;
};

struct MazeNode
{
	Vector3 _pos;

	bool _visited;

	int sizeOfNeighbour;

	MazeEdge neighbours[9];
};

class MazeGeneratorInfo3
{
public:
	int client_id;
	unsigned int Size;
	MazeNode start, end;
	MazeNode allNodes[400];
	MazeEdge allEdges[760];


	Vector3 finalPathCoords[100];
	Vector3 ExtendedfinalPathCoords[2000];
	int sizeA;
	int sizeB;
};

class MazeGenerator
{
public:
	MazeGenerator(); //Maze_density goes from 1 (single path to exit) to 0 (no walls at all)
	virtual ~MazeGenerator();

	void Generate(int size, float maze_density);

	void GenerateAnditionalData();

	//All points on the maze grid are connected in some shape or form
	// so any two nodes picked randomly /will/ have a path between them
	GraphNode* GetStartNode() const { return start; }
	GraphNode* GetGoalNode()  const { return end; }
	uint GetSize() const { return size; }


	//Used as a hack for the MazeRenderer to generate the walls more effeciently
	GraphNode* GetAllNodesArr() { return allNodes; }
	void GetUserDefineStartEnd(int x1, int y1, int x2, int y2);


protected:
	void GetRandomStartEndNodes();

	void Initiate_Arrays();

	void Generate_Prims();
	void Generate_Sparse(float density);

public:
	MazeGeneratorInfo3 data;
	uint size;
	GraphNode *start, *end;
	float density1;

	GraphNode* allNodes;
	GraphEdge* allEdges;

	void GraphEdgeToMazeEdge(GraphEdge a, MazeEdge& b)
	{
		b._iswall = a._iswall;
		b._connected = a._connected;
		b.weighting = a.weighting;
	}

	void GraphNodeToMazeNode(GraphNode a, MazeNode& b)
	{
		b._pos = a._pos;
		b._visited = a._visited;
		b.sizeOfNeighbour = a._neighbours.size();
		for (int i = 0; i < b.sizeOfNeighbour; ++i) {
			GraphEdgeToMazeEdge(*(a._neighbours[i]), b.neighbours[i]);
		}
	}

	void CollectGeneratorInfo(MazeGeneratorInfo3& a)
	{
		GraphNodeToMazeNode(*(start), a.start);
		GraphNodeToMazeNode(*(end), a.end);
		a.Size = size;
		for (int i = 0; i < size*size; ++i) {
			GraphNodeToMazeNode(allNodes[i], a.allNodes[i]);
		}

		for (int i = 0; i < size * (size - 1) * 2; ++i) {
			GraphEdgeToMazeEdge(allEdges[i], a.allEdges[i]);
		}
	}


	void MazeEdgeToGraphEdge(GraphEdge* a, MazeEdge* b)
	{
		a->weighting = b->weighting;
		a->_connected = b->_connected;
		a->_iswall = b->_iswall;
	}

	void MazeNodeToGraphNode(GraphNode a, MazeNode b)
	{
		a._pos = b._pos;
		a._visited = b._visited;
	}


	void GetMazeInfo()
	{
		size = data.Size;
		Initiate_Arrays();
		for (int i = 0; i < size * (size - 1) * 2; ++i) {
			allEdges[i].weighting = data.allEdges[i].weighting;
			allEdges[i]._connected = data.allEdges[i]._connected;
			allEdges[i]._iswall = data.allEdges[i]._iswall;
		}

		for (int i = 0; i < size * size; ++i) {
			allNodes[i]._pos = data.allNodes[i]._pos;
			allNodes[i]._visited = data.allNodes[i]._visited;
		}

		start = new GraphNode;
		end = new GraphNode;
		start->_pos = data.start._pos;
		start->_visited = data.start._visited;
		end->_pos = data.end._pos;
		end->_visited = data.end._visited;
	}
};




