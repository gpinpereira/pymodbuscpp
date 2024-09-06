#include <server_wrapper.h>
#include <chrono>
#include <thread>
#include <channel.h>

class Channel;


WServer::WServer(int iport){

    CUTIL::cMODBUSServer();
    port = iport;

    max_register = 2;

}


void WServer::addChannel(int index){

    Channel channel(0, 2, BOOL);

    channel.setBehaviour("Bsetpoint");
    channel.setServer(this);

    channels.push_back(channel);
}

void WServer::start(){


    std::string address= getLocalIP("127.0.0.1");

    std::cout << "port: " << port << std::endl;
    connect_TCP(address, port, 2);
    execute();

    while (true) {
        // Your code here (this block will run every second)
        updateChannels();
        // Pause execution for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    wait();
}


 void WServer::OnRequest(unsigned req_length)  {  // 'override' is optional but recommended for clarity
        //std::cout << "THA NEW REQUEST" << req_length << std::endl;
        uint8_t function_code = query()[7];

        switch (function_code) {
            case MODBUS_FC_WRITE_SINGLE_COIL: {
                // Write Single Coil (0x05)
                uint16_t coil_address = (query()[8] << 8) | query()[9];
                uint16_t coil_value = (query()[10] << 8) | query()[11];

                std::cout << "Received a write single coil request to coil: " << coil_address << std::endl;
                std::cout << "Coil value to write: " << (coil_value == 0xFF00 ? "ON" : "OFF") << std::endl;

                // Process the write coil request here...
                break;
            }

            case MODBUS_FC_WRITE_SINGLE_REGISTER: {
                // Write Single Register (0x06)
                uint16_t register_address = (query()[8] << 8) | query()[9];
                uint16_t value_to_write = (query()[10] << 8) | query()[11];

                std::cout << "Received a write single register request to register: " << register_address << std::endl;
                std::cout << "Value to write: " << value_to_write << std::endl;

                int int_val = static_cast<int>(value_to_write);  // Convert to int

                std::cout << "Value to write: " << int_val << std::endl;
                // Process the write register request here...
                break;
            }

            case MODBUS_FC_WRITE_MULTIPLE_COILS: {
                // Write Multiple Coils (0x0F)
                uint16_t starting_address = (query()[8] << 8) | query()[9];
                uint16_t quantity_of_coils = (query()[10] << 8) | query()[11];
                uint8_t byte_count = query()[12];

                std::cout << "Received a write multiple coils request starting at: " << starting_address << std::endl;
                std::cout << "Number of coils: " << quantity_of_coils << std::endl;
                std::cout << "Byte count: " << (int)byte_count << std::endl;

                // Data starts from query()[13], process coils data here...
                break;
            }

            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: {
                // Write Multiple Registers (0x10)
                uint16_t starting_address = (query()[8] << 8) | query()[9];
                uint16_t quantity_of_registers = (query()[10] << 8) | query()[11];
                uint8_t byte_count = query()[12];

                std::cout << "Received a write multiple registers request starting at: " << starting_address << std::endl;
                std::cout << "Number of registers: " << quantity_of_registers << std::endl;
                std::cout << "Byte count: " << (int)byte_count << std::endl;

                // Data starts from query[13], process registers data here...
                break;
            }

            default:
                //std::cout << "Received a request with function code: " << std::hex << (int)function_code << std::endl;
                break;
        }

        /*if (function_code == 0x05 ||  // Write Single Coil
            function_code == 0x06 ||  // Write Single Register
            function_code == 0x0F ||  // Write Multiple Coils
            function_code == 0x10) {  // Write Multiple Registers

            std::cout << "it is writing"  << std::endl;  

            uint16_t register_address = (query()[8] << 8) | query()[9];
            std::cout << "Received a write request to register: " << register_address << std::endl;

            if (function_code == 0x10) {
                uint16_t register_count = (query()[10] << 8) | query()[11];
                std::cout << "Number of registers to write: " << register_count << std::endl;
            }
  
        }*/
}


void WServer::updateChannels(){

    for(int i=0; i<channels.size(); i++){
        channels[i].updateMBValue();
    }

}