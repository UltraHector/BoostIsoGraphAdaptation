#include<string>
#include"AdjacenceListsGRAPH.h"
#include"StringUtility.h"
#include"GraphTransitiveReduction.h"
#include"FileUtility.h"
#include<fstream>
#include"TimeUtility.h"
using namespace std;


/**
* take one parameter which is the graph file name
*/
int main(int argc,char * argv[]) {

	GraphTransitiveReduction * adapter = new GraphTransitiveReduction(argv[1],  argv[2]);

	// start to record the time
	TimeUtility::StartCounterMill();
	adapter->computeContainmentGraphs();
	double timeClapseComputeContainment = TimeUtility::GetCounterMill();

	cout<<"** Done computing the containment graph: "<<timeClapseComputeContainment / (1000 * 60)<<" minutes !"<<endl;


	double totalTimeCost =  timeClapseComputeContainment;


	adapter->resultFile<<"Time cost to build "<<argv[1]<<" adaptaed graph is "<<totalTimeCost / (1000 * 60)<<" minutes!"<<endl;

	//system("pause");
	return 0;
}











