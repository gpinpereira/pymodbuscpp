#include "wrapper.h"
#include "channel.h"
#include <numeric> // For std::accumulate


Wrapper::Wrapper(){
    //readCSV();
	//processCSV();
}

void Wrapper::readCSV(char *filenamepath){

  std::string filename{filenamepath};
  std::ifstream input{filename};

  if (!input.is_open()) {
    std::cerr << "Couldn't read file: " << filename << "\n";
    return; 
  }


  for (std::string line; std::getline(input, line);) {
    std::istringstream ss(std::move(line));
    std::vector<std::string> row;
    if (!csvRows.empty()) {
       // We expect each row to be as big as the first row
      row.reserve(csvRows.front().size());
    }
    // std::getline can split on other characters, here we use ','
    for (std::string value; std::getline(ss, value, ',');) {
      row.push_back(std::move(value));
    }
    csvRows.push_back(std::move(row));
  }

}

void Wrapper::processCSV(){
	  // Print out our table

	int server_id_idx = 0;
	int server_name_idx = 1;
	int server_port_idx = 3;

	int channel_server_idx = 1;
	int channel_name_idx = 2;
	int channel_start_reg_idx = 6;
	int channel_n_reg_idx = 7;
	int channel_regtype_idx = 8;
	int channel_dtype_idx = 5;
	int channel_endian_idx = 4;
	int channel_behaviour_idx = 9;
	int channel_param_idx = 10;

	bool isAddingServers = false;
	bool isAddingChannels = false;

	

  	for (const std::vector<std::string>& row : csvRows) {

		//skip row if first value in row is empty
		if (cReplace(row[0], " ", "").size() == 0){
			continue;
		}

		if(isAddingServers && row[0] != "channelID"){
		
			int serverID = std::stoi(cReplace(row[server_id_idx], " ", ""));
			std::string serverName = cReplace(row[server_name_idx], " ", "");
			int serverPort = std::stoi(cReplace(row[server_port_idx], " ", ""));
			//std::cout << "Adding server: " << serverID << " " << serverName << " " << serverPort << "\n";

			WServer* server = new WServer();

			server->setName(serverName);
			server->setID(serverID);
			server->setPort(serverPort);

			addServer(server);

		}

		if(isAddingChannels){

			int serverID = std::stoi(cReplace(row[channel_server_idx], " ", ""));
			string name = row[channel_name_idx];

			
			int starting_reg = std::stoi(cReplace(row[channel_start_reg_idx], " ", ""));
			int n_reg = std::stoi(cReplace(row[channel_n_reg_idx], " ", ""));

			Rtype regtype = stringToRtype(cReplace(row[channel_regtype_idx], " ", ""));
			Dtype datatype = stringToDtype(cReplace(row[channel_dtype_idx], " ", ""));
			Endian endian = stringToEndian(cReplace(row[channel_endian_idx], " ", ""));

			string behaviour = cReplace(row[channel_behaviour_idx], " ", "");
			char* cbehaviour  = new char[behaviour.size() + 1];
			std::strcpy(cbehaviour, behaviour.c_str());

			std::vector<std::string> params_out = std::vector<std::string>(row.begin() + channel_param_idx, row.end());

			// Output the parameters in a single sentence
    		std::cout << "\tServer ID = " << serverID
					<< ", Name: " << name
					<< ", Starting Register = " << starting_reg
					<< ", Number of Registers = " << n_reg
					<< ", Register Type = " << RtypeToString(regtype)
					<< ", Data Type = " << DtypeToString(datatype)
					<< ", Endian = " << EndianToString(endian)
					<< ", Behaviour = " << cbehaviour << std::endl;

			Channel* channel = new Channel(starting_reg, n_reg, regtype, datatype, endian);
			
			channel->setBehaviour(cbehaviour, params_out);
			channel->setName(name);
			
			for(int i=0; i<servers_o.size(); i++){
				if(servers_o[i]->getID() == serverID){
					servers_o[i]->addChannel(channel);
				}
			}
		}


		if((row[0] == "serverID") && !isAddingServers ){
			isAddingServers = true;
		}

		if((row[0] == "channelID") && !isAddingChannels){
			isAddingChannels = true;
			isAddingServers = false;
			std::cout << "Added channels: " << std::endl;
		}

        
    }


}


void Wrapper::addServer(WServer *server){
	servers_o.push_back(server);
}

void Wrapper::printStatus(){
	std::cout << "Servers: "<< std::endl;

		for(int i=0; i<servers_o.size(); i++){
		std::cout << "\tId: " << servers_o[i]->getID()
				  << ", Port: " << servers_o[i]->getPort()
		          << ", N Channels: " << servers_o[i]->getChannels().size()
		          << ", Max holding: " << servers_o[i]->getMaxRegister()
		          << ", Max coil: " << servers_o[i]->getMaxCoil()
		          << ", Max input: " << servers_o[i]->getMaxInput()
		          << ", Max discrete: " << servers_o[i]->getMaxDiscrete() << std::endl;

	}

}


void Wrapper::start(){

	for(int i=0; i<servers_o.size(); i++){
		servers_o[i]->start();
	}

	servers_o[servers_o.size()-1]->wait();
	
}