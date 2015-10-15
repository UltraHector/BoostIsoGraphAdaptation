#ifndef GRAPH_ADAPTATION
#define GRAPH_ADAPTATION

#include <fstream>
#include<iostream>
#include<sstream>
#include<vector>
#include<map>
#include<string>
#include <algorithm>
#include"AdjacenceListsGRAPH_BOOST.h"
#include"AdjacenceListsGraph.h"
#include <boost/graph/adjacency_list.hpp> 
#include <boost/graph/transitive_reduction.hpp>

using namespace std;

class GraphAdaptation{

private:
	char * inputGraphName;
	char * outputHyperGraphName;
	char * outputContainmentGraphName;

	std::ifstream inputGraphFile;
	std::ofstream outputHypergraphFile;
	std::ifstream inputHypergraphFile;
	std::ofstream outputContainmentGraphFile;

	AdjacenceListsGRAPH * dataGraph;
	vector<AdjacenceListsGRAPH> dataGraphVector;
	AdjacenceListsGRAPH * hyperGraph;
	vector<AdjacenceListsGRAPH> hyperGraphVector;
	AdjacenceListsGRAPH * containmentGraph;
	vector<AdjacenceListsGRAPH> containmentGraphVector;

public:
	std::ofstream resultFile;
	GraphAdaptation(bool digraph, char* inputFilename, char * outputHyperFilename, char * outputContainmentGraphName);
	GraphAdaptation();
	~GraphAdaptation();
	/**
	* @para inputFilename the graph filename
	* @para outputFilename the filename which will store the hyper graph data and a file containing the containment relations
	*/

	void computeHyperGraphs();
	void loadHyperGraphs();
	void computeContainmentGraphs();
	

	void buildHyperGraphAlgorithm();
	void buildContainmentGraphAlgorithm();

	void computeTransitiveReduction(boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> &);

	bool isSyntacticEquivalent(int v, int u);

	bool isSyntacticContainment(int v, int u);

	void ouputStatistics();

private:
};



#endif