
#include <pybind11/pybind11.h>
#include <modbus_.h>

namespace py = pybind11;


class Channel {

public:
    Channel(int index); 

    void updateMBValue();
    void setBehaviour(char behaviour_name);

private:
    py::object behaviour;
    int mapping_index;
    cMODBUSServer *mb_server;

};
