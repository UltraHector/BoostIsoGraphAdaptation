#include"GraphAdaptation.h"
#include"StringUtility.h"
#include"AdjacenceListsGRAPH_IO.h"

using namespace std;



GraphAdaptation::GraphAdaptation(bool digraph, char* inputFilename, char * outputHyperFilename, char * outputContainmentFilename) {
	inputGraphName = inputFilename;
	outputHyperGraphName = outputHyperFilename;
	outputContainmentGraphName = outputContainmentFilename;

	inputGraphFile = std::ifstream(inputGraphName);
	outputHypergraphFile = std::ofstream(outputHyperGraphName);
	inputHypergraphFile = std::ifstream(outputHyperGraphName);
	outputContainmentGraphFile = std::ofstream(outputContainmentFilename);
	resultFile = std::ofstream("result.result", std::ios_base::app);

	if(!inputGraphFile.is_open()) {
		cout<<"data graph file doesn't exist"<<endl;
		exit(1);
	}
	
	// load the data graph
	AdjacenceListsGRAPH_IO::loadGraphFromFile(inputGraphFile, dataGraphVector);

	// build data graph label vertex label vertex index
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildVertexLabelVertexList();
	}
	// build data graph label vertex index
	/*for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildLabelVertexList();
	}*/
};

GraphAdaptation::~GraphAdaptation() {
	inputGraphFile.close();
	outputContainmentGraphFile.close();
	inputHypergraphFile.close();
}

void  GraphAdaptation::computeHyperGraphs(){
	
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		
		dataGraph = &dataGraphVector[dataGraphIndex];

		outputHypergraphFile<< "t # " <<dataGraphIndex<<endl;

		buildHyperGraphAlgorithm();
	}

	outputHypergraphFile.close();
	// release the memory of the original data graph
	// dataGraphVector.clear();
}

void  GraphAdaptation::loadHyperGraphs(){

	// load the hyper graph, only load the id and label
	AdjacenceListsGRAPH_IO::loadGraphFromFile(inputHypergraphFile, hyperGraphVector);

	// build hyper graph label vertex label vertex index
	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		hyperGraphVector[hyperGraphIndex].buildVertexLabelVertexList();
	}

	// build hyper graph label vertex index
	/*for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		hyperGraphVector[hyperGraphIndex].buildLabelVertexList();
	}*/

}

void GraphAdaptation::computeContainmentGraphs(){

	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		
		hyperGraph = &hyperGraphVector[hyperGraphIndex];

		outputContainmentGraphFile<< "t # " <<hyperGraphIndex<<endl;

		buildContainmentGraphAlgorithm();
	}

}


/**
* The containment graph shares the same set of data vertices with the hyper graph and is a transitive reduction of a DAG
*/
void GraphAdaptation::buildContainmentGraphAlgorithm(){

	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> BoostGraph;
	BoostGraph originalContainmentGraph;

	/**
	* compute containment graph
	*/
	for(int hyperVertexIndex = 0; hyperVertexIndex < hyperGraph->getNumberOfVertexes(); hyperVertexIndex++){

		AdjacenceListsGRAPH::Vertex* hyperVertex = hyperGraph -> getVertexAddressByVertexId(hyperVertexIndex);
		// iterate 1_step reacheability neighbours
		map<int, vector<int>>::iterator sameLabelNeighbourIterator = hyperVertex->labelVertexList.find(hyperVertex -> label);
		if(sameLabelNeighbourIterator != hyperVertex->labelVertexList.end()){
			for(vector<int>::iterator samelabelNeighbourIndex = sameLabelNeighbourIterator->second.begin(); samelabelNeighbourIndex != sameLabelNeighbourIterator->second.end(); samelabelNeighbourIndex++) {
				if(isSyntacticContainment(hyperVertex->id, *samelabelNeighbourIndex)){
					boost::add_edge (hyperVertex->id, *samelabelNeighbourIndex, originalContainmentGraph); 
				}
			}
		}
		// iterate 2_step reacheability neighbours
		AdjacenceListsGRAPH::adjIterator adjIterator(hyperGraph, hyperVertex->id);
		for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()) {

			map<int, vector<int>>::iterator _stepVertexIterator = hyperGraph->getVertexAddressByVertexId(t->v)->labelVertexList.find(hyperVertex->label);
			if(_stepVertexIterator != hyperGraph->getVertexAddressByVertexId(t->v)->labelVertexList.end()) {
				for(vector<int>::iterator samelabelNeighbourIndex = _stepVertexIterator->second.begin(); samelabelNeighbourIndex != _stepVertexIterator->second.end(); samelabelNeighbourIndex++){
					if(hyperVertex->id != *samelabelNeighbourIndex && isSyntacticContainment(hyperVertex->id, *samelabelNeighbourIndex)){
						boost::add_edge (hyperVertex->id, *samelabelNeighbourIndex, originalContainmentGraph); 
					}
				}
			}
		}

		// release memory
		//hyperVertex->labelVertexList.clear();
	}
	
	computeTransitiveReduction(originalContainmentGraph);
	
	/*
	* output the original containment graph. DO NOT DELETE
	*/
	/*BoostGraph::vertex_iterator vertexIt, vertexEnd;
	BoostGraph::adjacency_iterator neighbourIt, neighbourEnd;
	boost::tie(vertexIt, vertexEnd) = boost::vertices(originalContainmentGraph);

	for (; vertexIt != vertexEnd; ++vertexIt) {
		boost::tie(neighbourIt, neighbourEnd) = boost::adjacent_vertices(*vertexIt, originalContainmentGraph); 
		for (; neighbourIt != neighbourEnd; ++neighbourIt) {
			outputContainmentGraphFile<< "e " <<*vertexIt<<" "<<*neighbourIt<<std::endl;
		}
	}*/
}


void GraphAdaptation::computeTransitiveReduction(boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> & originalContainmentGraph){

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
	transitive_reduction(originalContainmentGraph, transitive_reduction_graph, t_to_tr_map, index);
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

/**
* v id label isCliqueFlag (vertex list) 
* isCliqueFlag
	0 means single vertex, 1 means a clique, 2 means not a clique 
*/
void GraphAdaptation::buildHyperGraphAlgorithm() {

	// only save one representative node into the hyoernode in memory
	std::map<int, int> hyperNode;
	int numberOfHyperVertex = 0;
	string compressedFileLine;

	bool * visitedFlag = new bool[dataGraph->getNumberOfVertexes()];
	int isCliqueFlag = 0;

	for(int i=0; i<dataGraph->getNumberOfVertexes(); i++){
		visitedFlag[i] = false;
	}

	for(int dataVertexIndex = 0; dataVertexIndex < dataGraph->getNumberOfVertexes(); dataVertexIndex++) {

		isCliqueFlag = false;

		if(visitedFlag[dataVertexIndex] == true){
			continue;
		}
		visitedFlag[dataVertexIndex] = true;

		AdjacenceListsGRAPH::Vertex* dataVertex = dataGraph -> getVertexAddressByVertexId(dataVertexIndex);

		compressedFileLine.clear();
		// create a hypervertex and output it 
		compressedFileLine.append("v ").append(std::to_string(static_cast<long long> (numberOfHyperVertex))).append(" ").append(std::to_string(static_cast<long long> (dataVertex->label))).append(" ");

		// iterate 1_step reacheability neighbours
		map<int, vector<int>>::iterator sameLabelNeighbourIterator = dataVertex->labelVertexList.find(dataVertex -> label);
		if(sameLabelNeighbourIterator != dataVertex->labelVertexList.end()){
			for(vector<int>::iterator samelabelNeighbourIndex = sameLabelNeighbourIterator->second.begin(); samelabelNeighbourIndex != sameLabelNeighbourIterator->second.end(); samelabelNeighbourIndex++){
				if(isSyntacticEquivalent(dataVertex->id, *samelabelNeighbourIndex)){
				 
					visitedFlag[*samelabelNeighbourIndex] = true;

					if(isCliqueFlag == 0) {
						isCliqueFlag = 1;
						// 1 represents a clique
						compressedFileLine.append("1 ");
						compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
					}

					compressedFileLine.append(std::to_string(static_cast<long long> (*samelabelNeighbourIndex))).append(" ");
				}
			}	
		}

		// if no nodes is found in the 1_step()
		if(isCliqueFlag == 0) {
		
			AdjacenceListsGRAPH::adjIterator adjIterator(dataGraph, dataVertex->id);
			for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()) {
				map<int, vector<int>>::iterator _stepVertexIterator = dataGraph->getVertexAddressByVertexId(t->v)->labelVertexList.find(dataVertex->label);
				if(_stepVertexIterator != dataGraph->getVertexAddressByVertexId(t->v)->labelVertexList.end()) {
					for(vector<int>::iterator samelabelNeighbourIndex = _stepVertexIterator->second.begin(); samelabelNeighbourIndex != _stepVertexIterator->second.end(); samelabelNeighbourIndex++){
					
						if(visitedFlag[*samelabelNeighbourIndex] == false && isSyntacticEquivalent(dataVertex->id, *samelabelNeighbourIndex)){
							visitedFlag[*samelabelNeighbourIndex] = true;

							if(isCliqueFlag == 0) {
								isCliqueFlag = 2;
								// 2 represents not a clique		
								compressedFileLine.append("2 ");
								compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
							}

							compressedFileLine.append(std::to_string(static_cast<long long> (*samelabelNeighbourIndex))).append(" ");
						}
					}
				}
			}
		}
		if(isCliqueFlag == 0){
			// 0 represents a single vertex		
			compressedFileLine.append("0 ");
			compressedFileLine.append(std::to_string(static_cast<long long> (dataVertexIndex))).append(" ");
		}
		// save a representative node into memory, which will be used to add edges
		hyperNode.insert(std::pair<int,int>(numberOfHyperVertex, dataVertexIndex));
		// ready to output
		compressedFileLine.append("\n");
		outputHypergraphFile << compressedFileLine;
		numberOfHyperVertex++;

		// release memory
		//dataVertex->labelSet.clear();
		//dataVertex->labelVertexList.clear();
	}

	// add edges into compressed group
	for(std::map<int, int>::iterator hyperNodeIterator = hyperNode.begin(); hyperNodeIterator != hyperNode.end(); hyperNodeIterator++){
		string compressedFileLine;

		std::map<int,int>::iterator loopIterator = hyperNodeIterator;
		int hyperNodeRepre =  (*hyperNodeIterator).second;
		loopIterator ++;
		while(loopIterator !=  hyperNode.end()){
			int a = 0;
			if(dataGraph->edge(hyperNodeRepre, (*loopIterator).second)){
				compressedFileLine.clear();
				// add the hypernode
				compressedFileLine.append("e ")
								  .append(std::to_string(static_cast<long long> (hyperNodeIterator->first)))
								  .append(" ")
								  .append(std::to_string(static_cast<long long> (loopIterator->first)))
								  .append(" 0").append("\n");
				outputHypergraphFile << compressedFileLine;
			}
			loopIterator++;
		}
	}

	delete [] visitedFlag;

}

bool GraphAdaptation::isSyntacticContainment(int v, int u){
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


bool GraphAdaptation::isSyntacticEquivalent(int v, int u){
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


void GraphAdaptation::ouputStatistics() {

	long numberOfOriginalVertices = 0;
	long numberOfHyperVertices = 0;
	long numberOfOriginalEgdes = 0;
	long numberOfHyperEgdes = 0;
	long numberOfContainmentRoots = 0;
	long numberOfContainmentEdges = 0;

	// build data graph label vertex label vertex index
	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		numberOfOriginalVertices = dataGraphVector[dataGraphIndex].getNumberOfVertexes();
		numberOfOriginalEgdes = dataGraphVector[dataGraphIndex].getNumberOfEdges();
	}

	for(int hyperGraphIndex = 0; hyperGraphIndex < hyperGraphVector.size(); hyperGraphIndex ++) {
		numberOfHyperVertices = hyperGraphVector[hyperGraphIndex].getNumberOfVertexes();
		numberOfHyperEgdes = hyperGraphVector[hyperGraphIndex].getNumberOfEdges();
	}

	std::ifstream inputContainmentGraphFile =  std::ifstream(outputContainmentGraphName);
	AdjacenceListsGRAPH_BOOST::loadContainmentGraph(hyperGraphVector, containmentGraphVector, inputContainmentGraphFile);
	AdjacenceListsGRAPH_BOOST::buildLabelRootMap(containmentGraphVector);

	for(int containmentGraphindex = 0; containmentGraphindex < containmentGraphVector.size(); containmentGraphindex ++) {
		map<int, vector<int>>::iterator rootListIterator = containmentGraphVector[containmentGraphindex].getLabelVertexList()->begin();
		for(; rootListIterator != containmentGraphVector[containmentGraphindex].getLabelVertexList()->end(); rootListIterator ++){
			numberOfContainmentRoots += rootListIterator->second.size();
		}
		numberOfContainmentEdges += containmentGraphVector[containmentGraphindex].getNumberOfEdges();
	}

	resultFile<<"numberOfOriginalVertices: "<<numberOfOriginalVertices<<endl;
	resultFile<<"numberOfHyperVertices: "<<numberOfHyperVertices<<endl;
	resultFile<<"numberOfOriginalEgdes: "<<numberOfOriginalEgdes<<endl;
	resultFile<<"numberOfOriginalVertices: "<<numberOfOriginalVertices<<endl;

	/*
	* compute the containment ratio
	*/
	resultFile<<"Size ratio: "<< (float)(numberOfHyperVertices + numberOfHyperEgdes + numberOfContainmentEdges)/ (numberOfOriginalVertices + numberOfOriginalEgdes)<<endl;
	resultFile<<"SC Ratio: "<<100 - (float) numberOfContainmentRoots / numberOfHyperVertices * 100<<endl;
	resultFile<<"Vertices ratio: "<<100 - (float) numberOfHyperVertices / numberOfOriginalVertices * 100<<endl;

}