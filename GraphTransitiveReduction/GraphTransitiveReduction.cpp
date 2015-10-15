#include"GraphTransitiveReduction.h"
#include"StringUtility.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<fstream>
#include<sstream>
#include <locale>

using namespace std;



GraphTransitiveReduction::GraphTransitiveReduction(char * inputcontainmentFilename, char * outputContainmentFilename) {

	outputContainmentGraphFile = std::ofstream(outputContainmentFilename);
	inputContainmentGraphFile = std::ifstream(inputcontainmentFilename);

};

GraphTransitiveReduction::~GraphTransitiveReduction() {
}


std::string extract_ints(std::ctype_base::mask category, std::string str, std::ctype<char> const& facet)
{
	using std::strlen;

	char const *begin = &str.front(),
		*end   = &str.back();

	auto res = facet.scan_is(category, begin, end);

	begin = &res[0];
	end   = &res[strlen(res)];

	return std::string(begin, end);
}

std::string extract_ints(std::string str)
{
	return extract_ints(std::ctype_base::digit, str,
		std::use_facet<std::ctype<char>>(std::locale("")));
}


void readIntegersFromString(string stringContents, std::vector<int>& numbers){
	int integerNumber;
	numbers.clear();
	std::stringstream ss(extract_ints(stringContents));
	while(ss>>integerNumber){
		numbers.push_back(integerNumber);
	}
}

void GraphTransitiveReduction::computeContainmentGraphs() {

		outputContainmentGraphFile<< "t # " <<0<<endl;

		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> BoostGraph;
		BoostGraph originalContainmentGraph;

		cout<<"1. Start loading containment graph ** "<<endl;
		string line;
		getline(inputContainmentGraphFile, line);
		getline(inputContainmentGraphFile, line);
		vector<int> integervalues;
		while(line.size() != 0){

			readIntegersFromString(line.substr(1), integervalues);
			boost::add_edge (integervalues[0], integervalues[1], originalContainmentGraph); 

			getline(inputContainmentGraphFile, line);
		}

		cout<<"2. Start computing transitive reduction ** "<<endl;
		computeTransitiveReduction(originalContainmentGraph);
}


void GraphTransitiveReduction::computeTransitiveReduction(boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> & originalContainmentGraph){

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> BoostGraph;
	BoostGraph transitive_reduction_graph;
	/**
	* compute containment graph transitive reduction
	*/
	typedef boost::property_map<BoostGraph, boost::vertex_index_t>::type IndexMap;
    IndexMap index = get(boost::vertex_index, originalContainmentGraph);

	int * t_to_tr_map = new int[num_vertices(originalContainmentGraph) + 1];
	int * tr_to_t_map = new int[num_vertices(originalContainmentGraph) + 1];
	
	if(num_vertices(originalContainmentGraph) == 0){
		return ;
	}
	for(int i=0; i<num_vertices(originalContainmentGraph);i++) {
		t_to_tr_map[i] = -1;
		tr_to_t_map[i] = -1;
	}

	cout<<"** 4.1 Boost library start computing transitive reduction ** "<<endl;
	transitive_reduction(originalContainmentGraph, transitive_reduction_graph, t_to_tr_map, index);
	
	cout<<"** 4.2 Done Boost library start computing transitive reduction ** "<<endl;
	// compute the tr_to_t map from t_to_tr map
	for(int i=0; i<num_vertices(originalContainmentGraph);i++) {
		for(int j=0; j<num_vertices(originalContainmentGraph);j++){
			if(t_to_tr_map[j] == i) {
				tr_to_t_map[i] = j;
			}
		}
	}


	/*
	* output the transitive reduction
	*/
	BoostGraph::vertex_iterator vertexIt, vertexEnd;
	BoostGraph::adjacency_iterator neighbourIt, neighbourEnd;
	boost::tie(vertexIt, vertexEnd) = boost::vertices(transitive_reduction_graph);

	for (; vertexIt != vertexEnd; ++vertexIt) { 

		boost::tie(neighbourIt, neighbourEnd) = boost::adjacent_vertices(*vertexIt, transitive_reduction_graph); 
		for (; neighbourIt != neighbourEnd; ++neighbourIt) {
			outputContainmentGraphFile<< "e " <<tr_to_t_map[*vertexIt]<<" "<<tr_to_t_map[*neighbourIt]<<std::endl;
		}
	}

	delete [] t_to_tr_map;
	delete [] tr_to_t_map;

}

bool GraphTransitiveReduction::isSyntacticContainment(int v, int u){
	AdjacenceListsGRAPH::Vertex* vVertex = hyperGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* uVertex = hyperGraph->getVertexAddressByVertexId(u);

	if(vVertex->inDegree <= uVertex->inDegree){
		return false;
	}

	if(vVertex->labelSet.size() < uVertex->labelSet.size()){
		return false;
	}

	for(map<int, vector<int>>::iterator ulabelIndex = uVertex->labelVertexList.begin(); ulabelIndex != uVertex->labelVertexList.end(); ulabelIndex++){
		map<int, vector<int>>::iterator vLabelIndex = vVertex->labelVertexList.find(ulabelIndex->first);
		if(vLabelIndex ==  vVertex->labelVertexList.end()){
			return false;
		}else if(vLabelIndex->second.size() < ulabelIndex->second.size()){
			return false;
		}
		else {
			vector<int>::iterator vVertexNeighbourIterator = vLabelIndex->second.begin();
			for(vector<int>::iterator uVertexNeighbourIterator = ulabelIndex->second.begin(); uVertexNeighbourIterator != ulabelIndex->second.end(); uVertexNeighbourIterator++){
				if(*uVertexNeighbourIterator == vVertex->id){
					continue;
				}
				while(*vVertexNeighbourIterator != *uVertexNeighbourIterator){
					vVertexNeighbourIterator ++;
					if(vVertexNeighbourIterator == vLabelIndex->second.end()){
						return false;
					}
				}
			}
		}
	
	}

	return true;

}


bool GraphTransitiveReduction::isSyntacticEquivalent(int v, int u){
	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* uVertex = dataGraph->getVertexAddressByVertexId(u);
	
	if(vVertex->inDegree != uVertex->inDegree){
		return false;
	}

	if(vVertex->labelSet.size() != uVertex->labelSet.size()){
		return false;
	}

	for(map<int, vector<int>>::iterator labelIndex = vVertex->labelVertexList.begin(); labelIndex != vVertex->labelVertexList.end(); labelIndex++){
		map<int, vector<int>>::iterator uVertexIterator = uVertex->labelVertexList.find(labelIndex->first);
		if(uVertexIterator ==  uVertex->labelVertexList.end())
			return false;
		if(labelIndex -> first == vVertex->label){
			if(labelIndex->second.size() != uVertexIterator->second.size()){
				return false;
			}else{
				for(int i=0,j=0;i < labelIndex->second.size() && j < uVertexIterator->second.size(); ){
					if(labelIndex->second[i] == uVertex->id){
						i++;
					}
					else if(uVertexIterator->second[j] == vVertex->id){
						j++;
					}
					else if(labelIndex->second[i] != uVertex->id && uVertexIterator->second[j] != vVertex->id){
						if(labelIndex->second[i] != uVertexIterator->second[j]){
							return false;
						}
						i++;
						j++;
					}
				}
			}
		}else{
			if(uVertexIterator->second != labelIndex->second)
				return false;
		}
	}
	
	return true;	
}
