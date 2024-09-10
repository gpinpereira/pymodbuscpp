#include "wrapper.h"
#include "channel.h"



Wrapper::Wrapper(){
    readCSV();
	processCSV();


	for(int i=0; i<servers_o.size(); i++){
		std::cout << "Server " << servers_o[i]->getID()  << std::endl;
		std::cout << "N Channels " << servers_o[i]->getChannels().size()  << std::endl;

		for(int j=0; j<servers_o[i]->getChannels().size(); j++){
			std::cout << "Starting reg " << servers_o[i]->getChannels()[j]->getStartingRegister()  << std::endl;

		}
		/*if(servers[i]->getID() == serverID){
			servers[i]->addChannel(channel);
		}*/
	}


}

void Wrapper::readCSV(){

  std::string filename{"config.csv"};
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
	int channel_start_reg_idx = 6;
	int channel_n_reg_idx = 7;
	int channel_regtype_idx = 8;
	int channel_dtype_idx = 5;
	int channel_endian_idx = 4;
	int channel_behaviour_idx = 9;

	bool isAddingServers = false;
	bool isAddingChannels = false;

	

  	for (const std::vector<std::string>& row : csvRows) {

		std::cout << servers_o.size() << " " << isAddingServers << " " << isAddingChannels << " " << row[0] << "\n";

		if (cReplace(row[server_id_idx], " ", "").size() == 0){
			continue;
		}

		if(isAddingServers && row[0] != "channelID"){
		
			int serverID = std::stoi(cReplace(row[server_id_idx], " ", ""));
			std::string serverName = cReplace(row[server_name_idx], " ", "");
			int serverPort = std::stoi(cReplace(row[server_port_idx], " ", ""));
			std::cout << "Adding server: " << serverID << " " << serverName << " " << serverPort << "\n";

			WServer* server = new WServer();

			server->setName(serverName);
			server->setID(serverID);
			server->setPort(serverPort);

			addServer(server);

		}

		if(isAddingChannels){

			int serverID = std::stoi(cReplace(row[channel_server_idx], " ", ""));
			int starting_reg = std::stoi(cReplace(row[channel_start_reg_idx], " ", ""));
			int n_reg = std::stoi(cReplace(row[channel_n_reg_idx], " ", ""));

			Rtype regtype = stringToRtype(cReplace(row[channel_regtype_idx], " ", ""));
			Dtype datatype = stringToDtype(cReplace(row[channel_dtype_idx], " ", ""));
			Endian endian = stringToEndian(cReplace(row[channel_endian_idx], " ", ""));

			string behaviour = cReplace(row[channel_behaviour_idx], " ", "");

			char* cbehaviour  = new char[behaviour.size() + 1];
			std::strcpy(cbehaviour, behaviour.c_str());

			// Output the parameters in a single sentence
    		std::cout << "Adding Modbus Channel: Server ID = " << serverID
					<< ", Starting Register = " << starting_reg
					<< ", Number of Registers = " << n_reg
					<< ", Register Type = " << RtypeToString(regtype)
					<< ", Data Type = " << DtypeToString(datatype)
					<< ", Endian = " << EndianToString(endian)
					<< ", Behaviour = " << cbehaviour << std::endl;

			Channel* channel = new Channel(starting_reg, n_reg, regtype, datatype, endian);
			channel->setBehaviour(cbehaviour);


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
		}

        
    }


}


void Wrapper::addServer(WServer *server){
	servers_o.push_back(server);

}
