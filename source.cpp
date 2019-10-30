#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <vector>
using namespace std;

// typedef for vector<vector<int>>
typedef vector<vector<int>> state;

// Node Struct
struct Node {
	Node(state* sp, Node* p = nullptr, string a = "")
		: sp(sp), parent(p), act(a), fn(0), gn(0), hn(0) {}
	state* sp;
	Node* parent;
	string act;
	int fn;
	int gn;
	int hn;
	pair<int, int> b1;
	pair<int, int> b2;
};

// typedefs for other types in use
typedef pair<int, Node*> nodeFn;
typedef priority_queue <nodeFn, vector<nodeFn>, greater<nodeFn>> pqOfStates;

// FUNCTION PROTOTYPES

// check if file is opened correctly
void openFile(ifstream& file);

// output to file function(s)
void outputToFile(
	Node* initial,
	Node* goal,
	int depth,
	int numOfNodes,
	vector<Node*> nodeSeq,
	ofstream& file);

// initialize the start and the goal
void init(ifstream& file, Node* res[], pair<int, int> goalPositions[]);

// determine position for a tile
void findPos(int tile, int res[], Node* node);

// determine positions for zeroes
void findZeroes(Node* node, int blank1[], int blank2[]);

// find Manhattan distance for a tile
int findManhattanDistance(int tile, Node* node, pair<int, int>& goalPos);

// h(n) heuristic function (sum of Manhattan distances)
int calcHeuristic(Node* node, pair<int, int> goalPositions[]);

// add state to frontier
void addFrontier(int prevGn, Node* newState, pair<int, int> goalPositions[], 
				 pqOfStates& frontier);

// check if state in explored
bool checkExplored(state* state, set<vector<vector<int>>>& explored);

// generate one child of a node given an action
Node* expand(const string& cmd, char blank, Node* curr);

// expand the next node and add to frontier
// note: we don't check frontier for b/c h(n) function is admissible
//		 and consistent
void action(
	const string& cmd, 
	nodeFn& currNodeFn,
	pqOfStates& frontier,		
	set<vector<vector<int>>>& explored,
	pair<int, int> goalPositions[]);

// run graph search with A*
Node* graphSearchwithAStar(
	Node* initial,
	Node* goal,
	pair<int, int> goalPositions[],
	const vector<string>& possibleActions,
	pqOfStates& frontier,
	set<vector<vector<int>>>& explored);

// trace solution path
void traceSolutionPath(Node* goal, vector<string>& solution,
					   const vector<string>& possibleActions);

int main() {
	// opening file
	ifstream file("test.txt");
	openFile(file);

	// initializing priority queue frontier (smallest f(n) will have highest
	// priority in the frontier
	pqOfStates frontier;

	// initializing the explored set
	set<vector<vector<int>>> explored;

	// initializing start and goal nodes
	pair<int, int>* goalPositions = new pair<int, int>[14];
	Node* arr[2];
	init(file, arr, goalPositions);
	Node* start = arr[0];
	Node* goal = arr[1];

	// initializing vector of all possible actions
	vector<string> possibleActions = { "U1", "U2", "L1", "L2", "D1", "D2",
									   "R1", "R2" };

	Node* goalNode = graphSearchwithAStar(start, goal, goalPositions, 
										  possibleActions, frontier, explored);

	vector<string> solution;
	traceSolutionPath(goalNode, solution, possibleActions);
	for (int i = solution.size() - 1; i >= 0; --i) {
		cout << solution[i] << endl;
	}
}
// NEED TO FIX ALL OTHER FUNCTIONS TO BE COMPATIBLE WITH NEW METHOD OF WORKING
// WITH BLANKS (store in Node)

// FUNCTION DEFINITIONS

// check open file
void openFile(ifstream& file) {
	string fileName;
	while (!file.is_open()) {
		cout << "File not found. Enter different file name:" << endl;
		cin >> fileName;
		file.open(fileName);
	}
}

// output to file function(s)
void outputToFile(
	Node* initial,
	Node* goal,
	int depth,
	int numOfNodes,
	vector<Node*> nodeSeq,
	ofstream& file) {}

// initialize the start and goal
void init(ifstream& file, Node* res[], pair<int, int> goalPositions[]) {
	state* startSp = new vector<vector<int>>(4, { 0,0,0,0 });
	state* goalSp = new vector<vector<int>>(4, { 0,0,0,0 });
	int tile;
	int blankPos1[2];
	int blankPos2[2];
	// set the start and goal states
	for (size_t row = 0; row < 4; ++row) {
		for (size_t col = 0; col < 4; ++col) {
			file >> tile;
			(*startSp)[row][col] = tile;
		}
	}
	for (size_t row = 0; row < 4; ++row) {
		for (size_t col = 0; col < 4; ++col) {
			file >> tile;
			(*goalSp)[row][col] = tile;
		}
	}

	// initalize nodes to contain start and goal states
	Node* start = new Node(startSp);
	Node* goal = new Node(goalSp);

	// setting goal positions for tiles
	for (size_t row = 0; row < 4; ++row) {
		for (size_t col = 0; col < 4; ++col) {
			if ((*goal->sp)[row][col] != 0) {
				goalPositions[(*goal->sp)[row][col] - 1].first = row;
				goalPositions[(*goal->sp)[row][col] - 1].second = col;
			}
		}
	}
	// setting attributes for starting node
	start->hn = calcHeuristic(start, goalPositions);
	start->gn = 0; // step cost for start = 0
	start->fn = start->hn;
	findZeroes(start, blankPos1, blankPos2);
	start->b1.first = blankPos1[0];
	start->b1.second = blankPos1[1];
	start->b2.first = blankPos2[0];
	start->b2.second = blankPos2[1];

	res[0] = start;
	res[1] = goal;
}

// determine position for a tile
void findPos(int tile, int res[], Node* node) {
	for (size_t row = 0; row < 4; ++row) {
		for (size_t col = 0; col < 4; ++col) {
			if ((*node->sp)[row][col] == tile) {
				res[0] = row;
				res[1] = col;
				return;
			}
		}
	}
}

// determine positions for zeroes
void findZeroes(Node* node, int blank1[], int blank2[]) {
	bool gotFirst = false;
	for (size_t row = 0; row < 4; ++row) {
		for (size_t col = 0; col < 4; ++col) {
			if ((*node->sp)[row][col] == 0) {
				if (!gotFirst) {
					blank1[0] = row;
					blank1[1] = col;
					gotFirst = true;
				}
				else {
					blank2[0] = row;
					blank2[1] = col;
				}
			}
		}
	}
}

// find Manhattan distance for a tile
int findManhattanDistance(int tile, Node* node, pair<int, int>& goalPos) {
	int manhattanDist = 0;
	int startRowCol[2];

	// get starting position
	findPos(tile, startRowCol, node);

	manhattanDist += abs(goalPos.first - startRowCol[0]);
	manhattanDist += abs(goalPos.second - startRowCol[1]);

	return manhattanDist;
}

// h(n) heuristic function (sum of Manhattan distances)
int calcHeuristic(Node* node, pair<int, int> goalPositions[]) {
	int heuristicVal = 0;
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			if ((*node->sp)[row][col] != 0) {
				heuristicVal += findManhattanDistance(
					(*node->sp)[row][col],
					node,
					goalPositions[(*node->sp)[row][col] - 1]);
			}
		}
	}
	return heuristicVal;
}

// add state to frontier (also assigns fn, hn, and gn)
void addFrontier(int prevGn, Node* newState, pair<int, int> goalPositions[], 
				 pqOfStates& frontier) {
	int hn = calcHeuristic(newState, goalPositions);
	newState->gn = prevGn + 1;
	newState->hn = hn;
	newState->fn = newState->gn + hn;
	frontier.emplace(newState->fn, newState);
}

// check if state in explored
bool checkExplored(state* state, set<vector<vector<int>>>& explored) {
	if (find(explored.begin(), explored.end(), *state) == explored.end()) {
		return false;
	}
	return true;
}

// generate one child of a node given an action
Node* expand(const string& cmd, char blank, Node* curr) {
	int row, col;
	if (blank == '1') {
		row = curr->b1.first; // row coordinate of blank
		col = curr->b1.second; // col coordinate of blank
	}
	else {
		row = curr->b2.first;
		col = curr->b2.second;
	}

	state* newSp = new vector<vector<int>>(*curr->sp); 
	Node* newState = new Node(newSp, curr, cmd);
	newState->b1.first = curr->b1.first;
	newState->b1.second = curr->b1.second;
	newState->b2.first = curr->b2.first;
	newState->b2.second = curr->b2.second;

	if (col != 0 && cmd[0] == 'L') {
		// don't swap if equal (ie. swapping blank positions)
		if ((*newState->sp)[row][col] != (*newState->sp)[row][col - 1]) {
			swap((*newState->sp)[row][col], (*newState->sp)[row][col - 1]);
			if (cmd[1] == '1') newState->b1.second -= 1;
			else if (cmd[1] == '2') newState->b2.second -= 1;
		}
	}
	else if (col != 3 && cmd[0] == 'R') {
		if ((*newState->sp)[row][col] != (*newState->sp)[row][col + 1]) {
			swap((*newState->sp)[row][col], (*newState->sp)[row][col + 1]);
			if (cmd[1] == '1') newState->b1.second += 1;
			else if (cmd[1] == '2') newState->b2.second += 1;
		}
	}
	else if (row != 0 && cmd[0] == 'U') {
		if ((*newState->sp)[row][col] != (*newState->sp)[row - 1][col]) {
			swap((*newState->sp)[row][col], (*newState->sp)[row - 1][col]);
			if (cmd[1] == '1') newState->b1.first -= 1;
			else if (cmd[1] == '2') newState->b2.first -= 1;
		}
	}
	else if (row != 3 && cmd[0] == 'D') {
		if ((*newState->sp)[row][col] != (*newState->sp)[row + 1][col]) {
			swap((*newState->sp)[row][col], (*newState->sp)[row + 1][col]);
			if (cmd[1] == '1') newState->b1.first += 1;
			else if (cmd[1] == '2') newState->b2.first += 1;
		}
	}
	else return nullptr;

	return newState;
}

// expand the next node and add to frontier
// note: we don't check frontier for b/c h(n) function is admissible
//		 and consistent
//void action(
//	const string& cmd,
//	pair<int, int> blank,
//	nodeFn& currNodeFn,
//	pqOfStates& frontier,
//	set<vector<vector<int>>>& explored, 
//	pair<int, int> goalPositions[]) {
//
//	int row = blank.first; // row coordinate of blank
//	int col = blank.second; // col coordinate of blank
//	//int temp;
//
//	Node* curr = currNodeFn.second;
//	int gn = currNodeFn.first - curr->hn;
//
//	explored.insert(*curr->sp);
//	if (cmd[0] == 'L' && col != 0) {
//		state* newSp = new vector<vector<int>>(*curr->sp);
//		Node* newState = new Node(newSp, curr);
//		swap((*newState->sp)[row][col], (*newState->sp)[row][col - 1]);
//		/*temp = (*newState->sp)[row][col];
//		(*newState->sp)[row][col] = (*newState->sp)[row][col - 1];
//		(*newState->sp)[row][col - 1] = temp;*/
//		if (!checkExplored(newSp, explored)) {
//			addFrontier(gn, newState, goalPositions, frontier);
//		}
//	}
//	else if (cmd[0] == 'R' && col != 3) {
//		state* newSp = new vector<vector<int>>(*curr->sp);
//		Node* newState = new Node(newSp, curr);
//		swap((*newState->sp)[row][col], (*newState->sp)[row][col - 1]);
//		/*temp = (*newState->sp)[row][col];
//		(*newState->sp)[row][col] = (*newState->sp)[row][col + 1];
//		(*newState->sp)[row][col + 1] = temp;*/
//		if (!checkExplored(newSp, explored)) {
//			addFrontier(gn, newState, goalPositions, frontier);
//		}
//	}
//	else if (cmd[0] == 'U' && row != 0) {
//		state* newSp = new vector<vector<int>>(*curr->sp);
//		Node* newState = new Node(newSp, curr);
//		swap((*newState->sp)[row][col], (*newState->sp)[row][col - 1]);
//		/*temp = (*newState->sp)[row][col];
//		(*newState->sp)[row][col] = (*newState->sp)[row - 1][col];
//		(*newState->sp)[row - 1][col] = temp;*/
//		if (!checkExplored(newSp, explored)) {
//			addFrontier(gn, newState, goalPositions, frontier);
//		}
//	}
//	else if (cmd[0] == 'D' && row != 3) {
//		state* newSp = new vector<vector<int>>(*curr->sp);
//		Node* newState = new Node(newSp, curr);
//		swap((*newState->sp)[row][col], (*newState->sp)[row][col - 1]);
//		/*temp = (*newState->sp)[row][col];
//		(*newState->sp)[row][col] = (*newState->sp)[row + 1][col];
//		(*newState->sp)[row + 1][col] = temp;*/
//		if (!checkExplored(newSp, explored)) {
//			addFrontier(gn, newState, goalPositions, frontier);
//		}
//	}
//
//}

void action(
	const string& cmd,
	nodeFn& currNodeFn,
	pqOfStates& frontier,
	set<vector<vector<int>>>& explored,
	pair<int, int> goalPositions[]) {

	explored.insert(*currNodeFn.second->sp);
	
	if (cmd[1] == '1') {
		Node* newState = expand(cmd, '1', currNodeFn.second);

		if (newState && !checkExplored(newState->sp, explored)) {
			addFrontier(currNodeFn.second->gn, newState, goalPositions, frontier);
		}
	}
	else if (cmd[1] == '2') {
		Node* newState = expand(cmd, '2', currNodeFn.second);

		if (newState && !checkExplored(newState->sp, explored)) {
			addFrontier(currNodeFn.second->gn, newState, goalPositions, frontier);
		}
	}

}

// run graph search with A*
Node* graphSearchwithAStar(
	Node* initial,
	Node* goal,
	pair<int, int> goalPositions[],
	const vector<string>& possibleActions,
	pqOfStates& frontier,
	set<vector<vector<int>>>& explored) {

	nodeFn currNodeFn;
	Node* curr = nullptr; 

	// construct fn and node pair for start
	nodeFn start(initial->fn, initial);

	// add start to frontier
	frontier.push(start);
	currNodeFn = frontier.top();
	curr = currNodeFn.second;

	// while we have not found the goal
	while (*curr->sp != *goal->sp && !frontier.empty()) {

		frontier.pop(); // remove node to be expanded from frontier

		// apply each action to current state and add to frontier
		for (const string& act : possibleActions) {
			if (act[1] == '1') {
				action(act, currNodeFn, frontier, explored, goalPositions);
			}
			else if (act[1] == '2') {
				action(act, currNodeFn, frontier, explored, goalPositions);
			}
		}

		currNodeFn = frontier.top();
		curr = currNodeFn.second;
	}
	return curr;
}

// trace solution path
void traceSolutionPath(Node* goal, vector<string>& solution,
					   const vector<string>& possibleActions) {

	Node* curr = goal;
	
	while (curr->parent) {
		solution.push_back(curr->act);
		curr = curr->parent;
	}
}