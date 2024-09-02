#include <channel.h>


class Server {

public:
    Server(int port); 

private:
    vector<Channel*> channels; 

};



