#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <map>
#include <string>
#include <assert.h>
#include <set>
#include <algorithm>
#include <random>
#include <chrono>
#include <time.h>

using namespace std;
typedef int PosF2[2];
typedef int Pos;

#define ZQY_DEBUG_PRINT 1

#define GET_ROW(pos,size)		((pos)/(size))  
#define GET_COL(pos,size)		(pos%size)
#define GET_IDX(row,col,size)	((row)*(size)+(col)) 

#define EMPTY  '0'
#define LIZARD '1'
#define TREE   '2'

typedef vector<char> T_NursaryLayout;

struct Cell
{
	char state; // '0' = empty, '1' = lizard; '2' = tree;
	int  attackCount;
};

// A node denoting the decision tree node
// set<Pos> denotes the unique identifier of the set
// Affected Positions denotes the positions affected by the current map
struct LizardPlacement 
{
	set<Pos>	lizardPos;
	set<Pos>	affectedPositions;		// This should be lazily evaluated
	set<Pos>	availablePositions;		// This should be needed to generate next set of item
	set<Pos>	availableRows;
	set<Pos>    availableCols;
	bool		evaluated;
};


bool comparePlacementOpt(LizardPlacement& A, LizardPlacement& B) { return min(A.availableCols.size(), A.availableRows.size()) < min(B.availableCols.size(), B.availableRows.size()); }
bool comparePlacement(LizardPlacement& A, LizardPlacement& B) { return A.availablePositions.size() < B.availablePositions.size(); }

class ZooKeeper
{
public:
	void parseInput();
	void generateOutput();
	void evaluateNursery(string mode = "DFS");
	void printLayout(T_NursaryLayout& layout, int size);
private:
	// Place the lizard on Position 
	bool placeLizard(Pos pos);
	// Remove the lizard from Position
	bool removeLizard(Pos pos);
	bool canKillEachOther(Pos Pos0, Pos Pos1, vector<Pos> trees);
	
	void SAInit();
	int  SACalculateAttackCount();
	void SAUpdate();
	double  SARandomProbability();
	inline bool isSameRow(Pos Pos0, Pos Pos1, int size)
	{
		return (floor(Pos0 / size) == floor(Pos1 / size));
	}
	inline bool isSameCol(Pos Pos0, Pos Pos1, int size)
	{
		return Pos0 % size == Pos1 % size;
	}
	inline bool isSameDiag(Pos Pos0, Pos Pos1, int size)
	{
		return abs(GET_ROW(Pos0,size) - GET_ROW(Pos1,size)) == abs(GET_COL(Pos0,size) - GET_COL(Pos1,size));
	}

	void evaluateAvailablePositions(LizardPlacement& placement);

	// Prebake affected positions based on layouts
	void computeAffectedPositions();
public:
	string m_Instruction;
	int    m_NurserySize;
	int	   m_NumLizards;
	T_NursaryLayout m_NurseryLayout;
	T_NursaryLayout m_FinalLayout;
	bool   m_foundResult;
	T_NursaryLayout m_WorkingLayout;
	set<Pos>		m_SALizardPositions;
	int				m_SATemperature;
	int				m_SAIterationCount;
	vector<Pos>		m_Trees; // a vector to store the set of trees present in the nursery
	unordered_map<Pos, vector<Pos>> m_precomputedAffectedPositions;
};

int main()
{
	ZooKeeper myKeeper;
	myKeeper.parseInput();
	myKeeper.evaluateNursery(myKeeper.m_Instruction);
	myKeeper.generateOutput();
	return 0;
}

void ZooKeeper::parseInput()
{

	std::ifstream inFile("input.txt");
	inFile >> m_Instruction;
	inFile >> m_NurserySize;
	inFile >> m_NumLizards;
	string temp;
	while (inFile >> temp)
	{
		assert(temp.size() == m_NurserySize);
		for(int i = 0; i < m_NurserySize; i++)
		{
			// original input
			m_NurseryLayout.push_back(temp[i]);//storing the input in the mNurseryLayout
			m_FinalLayout.push_back(temp[i]);
			// working map
			Cell cell;//generate a state node for this tree
			cell.state = temp[i];
			m_WorkingLayout.emplace_back(temp[i]);//emplace and not pushback as we dont need a memory copy
			
			// tree cache
			if (temp[i] == TREE)
			{
				m_Trees.push_back(m_NurseryLayout.size()-1);
			}
		}
	}
}

void ZooKeeper::generateOutput()
{
	std::ofstream outFile;
	outFile.open("output.txt");
	if (m_foundResult)
	{
		outFile << "OK" << endl;
		for (int i = 0; i < m_NurserySize; i++)
		{
			for (int j = 0; j < m_NurserySize; j++)
			{
				outFile << m_FinalLayout[GET_IDX(i,j,m_NurserySize)];
			}
			outFile << endl;
		}
	}
	else
	{
		outFile << "FAIL" << endl;
	}

}

void ZooKeeper::evaluateNursery(string mode)
{
	computeAffectedPositions();
	if (mode == "DFS")
	{
		// Initialized the stack for dfs
		stack<LizardPlacement> dfsStack;
		LizardPlacement node;
		node.evaluated = false;
		evaluateAvailablePositions(node);
		dfsStack.emplace(node);
		// Find the next available spot in the next row
		int lizardPlaced = 0;
		while (lizardPlaced < m_NumLizards && !dfsStack.empty())//try to convert to for loop
		{
			LizardPlacement currentPlacement = dfsStack.top();
			
			dfsStack.pop();
			if(!currentPlacement.availablePositions.empty()) // search depth
			{
				vector<LizardPlacement> tempList;
				for (Pos p : currentPlacement.availablePositions)
				{
					LizardPlacement newNode;
					newNode.lizardPos = currentPlacement.lizardPos; // Is this a copy constructor?
					newNode.lizardPos.insert(p);
					newNode.evaluated = false;
					evaluateAvailablePositions(newNode);					
					tempList.emplace_back(newNode);
				}
				std::sort(tempList.begin(), tempList.end(), comparePlacement);
				for (auto n : tempList)//try to use alternative of auto
				{
					dfsStack.push(n);//
				}
				lizardPlaced = dfsStack.top().lizardPos.size();
			}
			else // pop stack
			{
				lizardPlaced = currentPlacement.lizardPos.size();
			}
		}

		if(lizardPlaced == m_NumLizards)//solution found
		{
			assert(lizardPlaced == dfsStack.top().lizardPos.size());
			// write out put to working copy
			for (Pos p : dfsStack.top().lizardPos)
			{
				assert(m_FinalLayout[p] == EMPTY);
				m_FinalLayout[p] = LIZARD;
			}
			m_foundResult = true;//flag a result found
		}
		else
		{
			assert(dfsStack.empty());
			m_foundResult = false;
		}
	}
	else if (mode == "BFS")
	{
		//#TODO: BFS is super slow now, there need to be a mechanism to accelerate the methods
		//#QUSTION: is BFS searching all possible combinations in nth Lizard layer a good way?
		queue<LizardPlacement> bfsQueue;
		LizardPlacement node;
		node.evaluated = false;
		evaluateAvailablePositions(node);
		bfsQueue.emplace(node);

		int lizardPlaced = 0;
		while (lizardPlaced < m_NumLizards && !bfsQueue.empty())
		{
			LizardPlacement currentPlacement = bfsQueue.front();
			bfsQueue.pop();
			if (!currentPlacement.availablePositions.empty())
			{
				for(Pos p : currentPlacement.availablePositions)
				{
					LizardPlacement newNode;
					newNode.lizardPos = currentPlacement.lizardPos;
					newNode.lizardPos.insert(p);
					newNode.evaluated = false;
					evaluateAvailablePositions(newNode);
					bfsQueue.emplace(newNode);
				}
				vector<LizardPlacement> tempList;
				std::sort(tempList.begin(), tempList.end(), comparePlacement);
				for (auto n : tempList)
				{
					bfsQueue.push(n);
				}
				lizardPlaced = currentPlacement.lizardPos.size()+1;
			}
			else
			{
				lizardPlaced = currentPlacement.lizardPos.size();
			}
		}

		if (lizardPlaced == m_NumLizards)//solution found 
		{
			assert(lizardPlaced == bfsQueue.front().lizardPos.size());
			for (Pos p : bfsQueue.front().lizardPos)
			{
				assert(m_FinalLayout[p] == EMPTY);
				m_FinalLayout[p] = LIZARD;
			}
			m_foundResult = true;
		}
		else
		{
			assert(bfsQueue.empty());
			m_foundResult = false;
		}
	}
	else if (mode == "SA")
	{
		// #TODO: figure out deltaE and T for SA in this implementation
		// Set up the map with random 
		SAInit();

		while (m_SATemperature > 0)
		{
			SAUpdate();
			m_SAIterationCount++;
#if ZQY_DEBUG_PRINT
		//	cout << "Current temperature = " << m_SATemperature << endl;
		//	cout << "Current iteration count = " << m_SAIterationCount << endl;
#endif
		}
		if (m_SATemperature == 0)
		{
			m_foundResult = true;
			m_FinalLayout = std::move(m_WorkingLayout);
		}
	}
	else
	{
		// should not reach this point at all
		assert(false);
	}
}

void ZooKeeper::printLayout(T_NursaryLayout& layout, int size)
{
	assert(layout.size() == size*size);
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			cout << layout[GET_IDX(i, j, size)];
		}
		cout << endl;
	}
}

bool ZooKeeper::placeLizard(Pos pos)
{
	return true;
}

bool ZooKeeper::removeLizard(Pos pos)
{
	return true;
}

bool ZooKeeper::canKillEachOther(Pos Pos0, Pos Pos1, vector<Pos> trees)
{
	if (isSameRow(Pos0,Pos1,m_NurserySize)) // Same row
	{
		for (Pos treePos : trees)
		{
			if (isSameRow(Pos0, treePos, m_NurserySize) && (Pos0 - treePos) * (Pos1 - treePos) < 0) // Tree in between
			{
				return true;
			}
		}
	}
	else if(isSameCol(Pos0,Pos1,m_NurserySize)) // Same col
	{
		for (Pos treePos : trees)
		{
			if (isSameCol(Pos0, treePos, m_NurserySize) && (Pos0 - treePos) * (Pos1 - treePos) < 0)
			{
				return true;
			}
		}
	}
	else if (isSameDiag(Pos0,Pos1,m_NurserySize)) // Same diag
	{
		for (Pos treePos : trees)
		{
			if (isSameDiag(Pos0, treePos, m_NurserySize) && isSameDiag(Pos1, treePos, m_NurserySize) && (Pos0 - treePos) * (Pos1 - treePos) < 0)
			{
				return true;
			}
		}
	}
	else
	{
		return false;
	}
	return false;
}

void ZooKeeper::SAInit()
{
	// This function places the lizards randomly inside the working space
	// Then calculates the current T
	for (int i = 0; i < m_NumLizards; i++)
	{
		srand(i);
		int row = i % m_NurserySize;
		int col = rand() % m_NurserySize;
		int idx = GET_IDX(row, col, m_NurserySize);
		while (m_WorkingLayout[idx] != EMPTY)
		{
			idx = (idx + 1) % (m_NurserySize*m_NurserySize);
		}
		m_WorkingLayout[idx] = LIZARD;
		m_SALizardPositions.insert(idx);
	}
	// calculate the conflict score T every turn
	m_SATemperature = SACalculateAttackCount();
	m_SAIterationCount = 0;
#if ZQY_DEBUG_PRINT
//	printLayout(m_WorkingLayout, m_NurserySize);
#endif
}

int ZooKeeper::SACalculateAttackCount()
{
	int count = 0;
	for (auto pos : m_SALizardPositions)
	{
		for (auto attackedPos : m_precomputedAffectedPositions[pos])
		{
			if (m_SALizardPositions.find(attackedPos) != m_SALizardPositions.end())
			{
				count++;
			}
		}
	}
#if ZQY_DEBUG_PRINT
//	cout << "Attack count: " << count << endl;
#endif
	return count;
}

void ZooKeeper::SAUpdate()
{
	// pure randomized selection of movement converges slowly.
	// Add a selection for the most promising move?
	// What heuristics is needed?
	auto itLizard(m_SALizardPositions.begin());
	double advancement = SARandomProbability() * m_SALizardPositions.size();
	advance(itLizard, floor(advancement));
	int randEmptyPos = floor(SARandomProbability() * (m_NurserySize*m_NurserySize));
	int curPos = *itLizard;
	
	while (m_WorkingLayout[randEmptyPos] != EMPTY)
	{
		randEmptyPos = (randEmptyPos+1) % (m_NurserySize*m_NurserySize);
	}
	int curAttackCount = 0;
	int newAttackCount = 0;
	for (auto pos : m_precomputedAffectedPositions[curPos])
	{
		if (m_SALizardPositions.find(pos) != m_SALizardPositions.end() && pos != curPos) 
		{
			curAttackCount+=2;
		}
	}
	for (auto pos : m_precomputedAffectedPositions[randEmptyPos])
	{
		if (m_SALizardPositions.find(pos) != m_SALizardPositions.end() && pos != curPos)
		{
			newAttackCount+=2;
		}
	}
	srand(time(nullptr));
#if ZQY_DEBUG_PRINT
	//cout << "Current Attack Count is" << curAttackCount << " at " << curPos << endl;
	//cout << "Current Attack Count is" << newAttackCount << " at " << randEmptyPos << endl;
#endif
	if (newAttackCount < curAttackCount)
	{
		// accept 
		m_SATemperature = m_SATemperature - curAttackCount + newAttackCount;

		m_WorkingLayout[randEmptyPos] = LIZARD;
		m_WorkingLayout[curPos] = EMPTY;
		m_SALizardPositions.insert(randEmptyPos);
		m_SALizardPositions.erase(curPos);
#if ZQY_DEBUG_PRINT
	//	cout << "Accepted move from " << curPos << " to " << randEmptyPos << endl;
#endif
	}
	else
	{
		double pAccept = exp(((double)curAttackCount - (double)newAttackCount) / (double)m_SATemperature);
		if (SARandomProbability() < pAccept)
		{
			// accept with probability
			m_SATemperature = m_SATemperature - curAttackCount + newAttackCount;
			m_WorkingLayout[randEmptyPos] = LIZARD;
			m_WorkingLayout[*itLizard] = EMPTY;
			m_SALizardPositions.insert(randEmptyPos);
			m_SALizardPositions.erase(*itLizard);
#if ZQY_DEBUG_PRINT
	//		cout << "Accepted move from " << curPos << " to " << randEmptyPos << "with probability" << pAccept << endl;
#endif
		}
		else
		{
#if ZQY_DEBUG_PRINT
	//		cout << "Rejected move from " << curPos << " to " << randEmptyPos << "with probability" << pAccept << endl;
#endif
		}
	}
#if ZQY_DEBUG_PRINT
	printLayout(m_WorkingLayout, m_NurserySize);
#endif
}

double ZooKeeper::SARandomProbability()//check and see what all these methods mean
{
	mt19937_64 rng;
	uint64_t timeSeed = chrono::high_resolution_clock::now().time_since_epoch().count();
	seed_seq ss{ uint32_t(timeSeed & 0xffffffff),uint32_t(timeSeed >> 32) };
	rng.seed(ss);
	std::uniform_real_distribution<double> unif(0, 1);
	double res = unif(rng);
#if ZQY_DEBUG_PRINT
	//cout << "Random number generated = " << res << endl;
#endif
	return res;
}


void ZooKeeper::evaluateAvailablePositions(LizardPlacement& placement)
{
	if (!placement.evaluated)
	{
		for (auto p : placement.lizardPos)
		{
			vector<Pos>& affectedPos = m_precomputedAffectedPositions.find(p)->second;
			for (auto pp : affectedPos)
			{
				placement.affectedPositions.insert(pp);
			}	
		}

		for (int i = 0; i < m_NurserySize*m_NurserySize; i++)
		{
			if (m_NurseryLayout[i] == EMPTY
				&& placement.affectedPositions.find(i) == placement.affectedPositions.end()
				&& placement.lizardPos.find(i) == placement.lizardPos.end())
			{
				placement.availablePositions.insert(i);
			}
		}
		for (Pos p : placement.availablePositions)
		{
			placement.availableCols.insert(GET_COL(p, m_NurserySize));
			placement.availableRows.insert(GET_ROW(p, m_NurserySize));
		}
		placement.evaluated = true;
	}
}

void ZooKeeper::computeAffectedPositions()
{
	// Question: what's the cost and benefit of writing this? If trees are very few, there is no point, just check if trees block between the two

	for (int i = 0; i < m_NurserySize * m_NurserySize; i++)
	{
		vector<Pos> attackingRange;
		 // Mark Row cells attacked by from i
		{
			int col = GET_COL(i,m_NurserySize);
			int row = GET_ROW(i, m_NurserySize);
			int minTree = row * m_NurserySize - 1;
			int maxTree = (row + 1) * m_NurserySize;
			for (int j = row * m_NurserySize; j < (row+1)* m_NurserySize; j++)
			{
				//#TODO: can be optimized and shorter
				if (j < i && m_NurseryLayout[j] == TREE && minTree < j)
				{
					minTree = j;
				}
				else if (j > i && m_NurseryLayout[j] == TREE && maxTree > j)
				{
					maxTree = j;
				}
			}
			for (int j = minTree + 1; j < maxTree; j++)
			{
				if(j!=i)
				{		
					attackingRange.push_back(j);
				}
			}

		}

		// Mark Col cells attacked by from i
		{
			int row = GET_ROW(i, m_NurserySize);
			int col = GET_COL(i, m_NurserySize);
			int minTree = -m_NurserySize + col;
			int maxTree = m_NurseryLayout.size();
			// Test all the trees in this col
			for (int j = col; j < m_NurserySize * m_NurserySize + col; j += m_NurserySize)
			{
				//#TODO: can be optimized and shorter
				if (j < i && m_NurseryLayout[j] == TREE && minTree < j)
				{
					minTree = j;
				}
				else if (j > i && m_NurseryLayout[j] == TREE && maxTree > j)
				{
					maxTree = j;
				}
			}
			// store attacking rows from pos i
			for (int j = minTree + m_NurserySize; j < maxTree; j += m_NurserySize)
			{
				if(j!=i)
				{
					attackingRange.push_back(j);
				}
			}
		}

		
		// Mark \ diagnal
		{
			int row = GET_ROW(i, m_NurserySize)-1;
			int col = GET_COL(i, m_NurserySize)-1;
			while (row >= 0 && col >= 0)
			{
				int idx = GET_IDX(row, col, m_NurserySize);
				if (m_NurseryLayout[idx] != TREE)
				{
					attackingRange.push_back(idx);
					row--;
					col--;
				}
				else
				{
					break;
				}
			}
			row = GET_ROW(i, m_NurserySize) + 1;
			col = GET_COL(i, m_NurserySize) + 1;
			while (row < m_NurserySize && col < m_NurserySize)
			{
				int idx = GET_IDX(row, col, m_NurserySize);
				if (m_NurseryLayout[idx] != TREE)
				{
					attackingRange.push_back(idx);
					row++;
					col++;
				}
				else
				{
					break;
				}
			}
		}

		// Mark / diagnal
		{
			int row = GET_ROW(i, m_NurserySize) - 1;
			int col = GET_COL(i, m_NurserySize) + 1;
			while (row >= 0 && row < m_NurserySize && col >= 0 && col < m_NurserySize)
			{
				int idx = GET_IDX(row, col, m_NurserySize);
				if (m_NurseryLayout[idx] != TREE)
				{
					attackingRange.push_back(idx);
					row--;
					col++;
				}
				else
				{
					break;
				}
			}
			row = GET_ROW(i, m_NurserySize) + 1;
			col = GET_COL(i, m_NurserySize) - 1;
			while (row >= 0 && row < m_NurserySize && col >= 0 && col < m_NurserySize)
			{
				int idx = GET_IDX(row, col, m_NurserySize);
				if (m_NurseryLayout[idx] != TREE)
				{
					attackingRange.push_back(idx);
					row++;
					col--;
				}
				else
				{
					break;
				}
			}
		}
		m_precomputedAffectedPositions.emplace(std::pair<Pos, vector<Pos>>(i,attackingRange));
	}
}
