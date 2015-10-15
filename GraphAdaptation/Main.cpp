#include<string>
#include"AdjacenceListsGRAPH.h"
#include"StringUtility.h"
#include"GraphAdaptation.h"
#include"FileUtility.h"
#include<fstream>
#include"TimeUtility.h"
using namespace std;


/**
* take one parameter which is the graph file name
*/
int main(int argc,char * argv[]) {

	if(argc != 4) {
		std::cout<<"File name specified error."<<endl<<"Example: GraphAdaptation sourceFilename destFilename, containmentFileName, numberOfLabels" << endl;
		system("pause");
		return 0;
	}

	GraphAdaptation * adapter = new GraphAdaptation(false, argv[1],  argv[2], argv[3]);
	cout<<"** 1. Done loading the data graph **"<<endl;
	
	// start to record the time
	TimeUtility::StartCounterMill();
	adapter->computeHyperGraphs(); 
	double timeClapseComputeHyper = TimeUtility::GetCounterMill();

	cout<<"** 2. Done computing the hypergraph" <<timeClapseComputeHyper / (1000 * 60)<<" minutes !"<<endl;

	TimeUtility::StartCounterMill();
	adapter->loadHyperGraphs();
	double timeClapseLoadHyper = TimeUtility::GetCounterMill();

	cout<<"** 3. Done loading the hypergraph "<<timeClapseLoadHyper / (1000 * 60)<<" minutes !"<<endl;

	// start to record the time
	TimeUtility::StartCounterMill();
	adapter->computeContainmentGraphs();
	double timeClapseComputeContainment = TimeUtility::GetCounterMill();

	cout<<"** 4. Done computing the containment graph: "<<timeClapseComputeContainment / (1000 * 60)<<" minutes !"<<endl;


	double totalTimeCost = timeClapseComputeHyper + timeClapseComputeContainment;


	adapter->resultFile<<"Time cost to build "<<argv[1]<<" adaptaed graph is "<<totalTimeCost / (1000 * 60)<<" minutes!"<<endl;
	
	adapter->ouputStatistics();

	//system("pause");
	return 0;
}











