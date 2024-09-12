#include <server_wrapper.h>
#include <chrono>
#include <thread>
#include <channel.h>

#include <pybind11/pybind11.h>
//#include <pybind11/embed.h>  // python interpreter
#include <pybind11/stl.h>  // type conversion
#include "server_wrapper.h"

namespace py = pybind11;


class Channel;


WServer::WServer(int iport){

    CUTIL::cMODBUSServer();
    port = iport;
}

WServer::WServer(){

    CUTIL::cMODBUSServer();
}

void WServer::addChannel(Channel *channel){

    int last_reg = channel->getStartingRegister() + channel->getTotalRegister();
    Rtype rtype = channel->getRegisterType();

    if (rtype == HOLDINGREGISTER) {
        if(last_reg>getMaxRegister())
            setMaxRegister(last_reg);

    } else if (rtype == INPUTREGISTER) {
        if(last_reg>getMaxInput())
            setMaxInput(last_reg);

    } else if (rtype == COIL) {
        if(last_reg>getMaxCoil())
            setMaxCoil(last_reg);
    } else if (rtype == DESCRETEINPUT) {
        if(last_reg>getMaxDiscrete())
            setMaxDiscrete(last_reg);
    }

    channel->setServer(this);
    channels.push_back(channel);
}

void WServer::start(){

    std::string address= getLocalIP("127.0.0.1");

    std::cout << "Started serving server "<< getID() <<" on port: " << port << std::endl;
    connect_TCP(address, port, 2);
    execute();

    while (true) {
        updateChannels();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


 void WServer::OnRequest(unsigned req_length)  {  // 'override' is optional but recommended for clarity
        //std::cout << "THA NEW REQUEST" << req_length << std::endl;
        uint8_t function_code = query()[7];
        uint16_t reg_address;
        std::vector<uint16_t> reg_values;
        Rtype rtype;

        if(function_code == MODBUS_FC_WRITE_SINGLE_COIL){

            // Write Single Coil (0x05)
            rtype = COIL;
            reg_address = (query()[8] << 8) | query()[9];
            uint16_t coil_value = (query()[10] << 8) | query()[11];
            reg_values.push_back(coil_value);

        } else if(function_code == MODBUS_FC_WRITE_SINGLE_REGISTER){
            // Write Single Register (0x06)
            rtype = HOLDINGREGISTER;
            reg_address = (query()[8] << 8) | query()[9];
            uint16_t value_to_write = (query()[10] << 8) | query()[11];
            reg_values.push_back(value_to_write);

        } else if(function_code == MODBUS_FC_WRITE_MULTIPLE_COILS){
            // Write Multiple Coils (0x0F)
            rtype = COIL;
            reg_address = (query()[8] << 8) | query()[9];
            uint16_t num_coils = (query()[10] << 8) | query()[11];
            //uint8_t byte_count = query()[12];

            for (int i = 0; i < num_coils; i++) {
                int byte_index = 13 + (i / 8);  // Start of data + byte offset
                int bit_position = i % 8;       // Position of the bit within the byte

                // Extract the current coil value (0 or 1)
                uint8_t coil_value = (query()[byte_index] >> bit_position) & 0x01;
                reg_values.push_back(coil_value);
            }

            // Data starts from query()[13], process coils data here...
        } else if(function_code == MODBUS_FC_WRITE_MULTIPLE_REGISTERS){
            // Write Multiple Registers (0x10)

            rtype = HOLDINGREGISTER;
            reg_address  = (query()[8] << 8) | query()[9];
            uint16_t num_registers  = (query()[10] << 8) | query()[11];

            for (int i = 0; i < num_registers; i++) {
                uint16_t value = (query()[13 + (i * 2)] << 8) | query()[14 + (i * 2)];
                reg_values.push_back(value);
            }

        }


        if(reg_values.size()>0){

            for(int i=0; i<channels.size(); i++){

                if(channels[i]->getStartingRegister() == reg_address &&
                    channels[i]->getRegisterType() == rtype)
                    channels[i]->setBehaviourValue(reg_values);
            }
        }
}


void WServer::updateChannels(){

    for(int i=0; i<channels.size(); i++){
        channels[i]->updateValue();
    }
}


Channel* WServer::getChannel(std::string name){

    for(int i=0; i<channels.size(); i++){
        if(channels[i]->getName() == name){
            return channels[i];
        }
    }
    
    return nullptr;
}

