#ifndef WWrapper_H
#define WWrapper_H

#include <server_wrapper.h>


using namespace CUTIL;
using namespace std;



class Wrapper {

public:
    Wrapper();
	void readCSV(char *filenamepath);
	void processCSV();
	void printStatus();
	void start();
private:
	vector<vector<string>> csvRows;

	void addServer(WServer *server);

	std::vector<WServer*> servers_o;
	
};


#endif // WServer_H