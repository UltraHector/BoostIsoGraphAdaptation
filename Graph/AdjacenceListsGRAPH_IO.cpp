#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include"StringUtility.h"


using namespace std;


void AdjacenceListsGRAPH_IO::show(const AdjacenceListsGRAPH* G){
	for(int s=0; s < G->getNumberOfVertexes(); s++){
		std::cout.width(2);
		std::cout<< s << ":";
		AdjacenceListsGRAPH::adjIterator A(G,s);
		for(AdjacenceListsGRAPH::link t=A.begin();  !A.end(); t = A.next()){
			std::cout.width(2);
			std::cout<< t->v << " ";
		}
		std::cout << std::endl;
	}
};




void AdjacenceListsGRAPH_IO::loadGraphFromFile(std::ifstream & graphFile, std::vector<AdjacenceListsGRAPH> & graphList) {
	// graph data loaded into memory
	string line;
	vector<int> integerValues;

	getline (graphFile,line);
	while ( line.size() != 0 && (*line.begin()) == 't'){
		// A new graph
		graphList.push_back(AdjacenceListsGRAPH(false));
		AdjacenceListsGRAPH & graph = *(graphList.begin() + graphList.size() - 1);
		// insert all the nodes
		while(getline (graphFile,line) && (*line.begin() == 'v')){
			String_Utility::readIntegersFromString(line,integerValues);
			// only consider one label per vertex
			AdjacenceListsGRAPH::Vertex vertex =  AdjacenceListsGRAPH::Vertex(*integerValues.begin(),*(integerValues.begin()+1));

			/****************************************/
			/* add the vertexList if a hyper vertex */
			/****************************************/
			if(integerValues.begin()+2 != integerValues.end()){
				graph.isHyperGraph = true;
				// hyper vertex starts from the second element of this vector, the first element is a flag to mark it as clique
				vertex.isClique = *(integerValues.begin()+2);
				for(vector<int>::iterator vertexListIterator = integerValues.begin() + 3; vertexListIterator != integerValues.end(); vertexListIterator++){
					vertex.vertexList.push_back(*vertexListIterator);	
				}
			}
			graph.insert(vertex);
		}
		// insert all the edges
		do{
			String_Utility::readIntegersFromString(line, integerValues);
			if(*integerValues.begin() != *(integerValues.begin()+1)){
				AdjacenceListsGRAPH::Edge edge =  AdjacenceListsGRAPH::Edge(*integerValues.begin(),*(integerValues.begin()+1),*(integerValues.begin()+2));
				graph.insert(edge);
			}
		}while(getline (graphFile,line) && (*line.begin() == 'e'));
		// insert the query graph into query list	
	}
}
