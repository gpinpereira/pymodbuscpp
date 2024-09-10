#ifndef WWrapper_H
#define WWrapper_H

#include <server_wrapper.h>


using namespace CUTIL;
using namespace std;



class Wrapper {

public:
    Wrapper();
	

private:
	vector<vector<string>> csvRows;
	void readCSV();
	void processCSV();
	void addServer(WServer *server);

	std::vector<WServer*> servers_o;
	
};


#endif // WServer_H