# ModbusWrapperCpp

# Modbus Server Wrapper in C++

This repository provides a C++ wrapper for generating Modbus servers, designed to simplify the configuration and management of Modbus channels from a CSV file. It uses **pybind11** to integrate custom Python scripts for handling the `get` and `set` operations of channels, facilitating easy server implementation and customization in Python.

## Key Features:
- **CSV-based Configuration**: Channels can be set up and configured through a CSV file for quick and flexible deployment.
- **Python Integration**: Via **pybind11**, Python scripts can be used to manage channel operations, offering flexibility in defining how channel values are retrieved and updated.
- **Modbus Server Generation**: Quickly generate and launch Modbus servers with this C++ wrapper, streamlining server setup.

This repository is ideal for those looking to implement Modbus servers with a combination of C++ performance and Python's flexibility.

## Usage


### Configuring the Modbus Server from CSV

This repository allows you to configure Modbus servers based on a CSV file structure. Below are the steps to set up and manage your servers and channels using the CSV configuration.

### CSV Structure

The CSV file used for configuration consists of two main sections: server information and channel details.

#### Server Configuration Section

The server is defined in the first part of the CSV file:

```csv
serverID,Name,Description,Port,
1 ,wrapper,,502,
```
- serverID: Unique identifier for the Modbus server.
- Name: Name of the server.
- Port: The port number where the server will listen (e.g., 502 for Modbus TCP).

#### Channel Configuration Section

Channels associated with the Modbus server are defined in the following part:

```csv
channelID,serverID,Name,Description,Reverse word order,Channel Datatype,MB starting add,MB length,MB type,Behavior,Command
1,1,Ch 1.1.0,,LITTLE,BOOL,0,1,COIL,Bsetpoint,0,1
2,1,Ch 1.1.1,,LITTLE,BOOL,1,1,COIL,Bsetpoint,0,1
3,1,Ch 1.1.2,,LITTLE,FLOAT,2,2,HOLDING_REGISTER,Bsetpoint,0,1
```

Each channel entry includes:

- **channelID**: Unique identifier for the channel.
- **serverID**: The server that this channel belongs to.
- **Name**: Name of the channel.
- **Reverse word order**: The byte order of the data (e.g., LITTLE).
- **Channel Datatype**: The datatype of the channel (e.g., BOOL, FLOAT, INTEGER).
- **MB starting add**: Modbus starting address for this channel.
- **MB length**: Length of the data in Modbus registers.
- **MB type**: Type of Modbus data (e.g., COIL, HOLDING_REGISTER).
- **Behavior**: The behavior associated with the channel (`Bsetpoint`, `Bcopy`, `Bsinwave`).
- **Command**: Optional commands or parameters for the behavior.

### Reimplementing Abstract Methods in Behavior Examples

In the Python behavior classes, the following abstract methods are reimplemented to define how each behavior manages channel values (see `Behaviours.py`):

1. **`updateValue()`**: This method defines how the channel’s value is updated over time:
   - **Bsetpoint**: Generates a random value within a range based on a constant.
   - **Bcopy**: Copies the value from a specified reference channel.
   - **Bsinwave**: Produces a sine wave pattern to update the channel’s value periodically.

2. **`getValue()`**: Returns the current value of the behavior and is internally called by the server to export the value to the modbus side, and finally, the master. 

3. **`setValue(value)`**: Allows setting a new value manually. This function is called when there is a write operation from the master:
   - **Bsetpoint**: Updates the constant used for generating new random values.
   - **Bcopy** and **Bsinwave**: The behavior does not allow manual setting, as their values are dynamically calculated.


## Installation


Compile the Code
```bash
make
```
and run:
```bash
sudo ./wrapper
```

## License
For open source projects, say how it is licensed.

## Project status
The project is **ready to use**, but it is **under development**. Feedback and contributions are welcome as I continue to improve the project.
