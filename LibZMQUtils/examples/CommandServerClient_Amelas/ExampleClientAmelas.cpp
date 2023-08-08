

#include <iostream>
#include <cstring>


// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Helpers>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasController/common.h"
#include "AmelasServer/common.h"
#include "AmelasClient/amelas_client.h"
// =====================================================================================================================


// TODO Remove
using namespace zmqutils;
using namespace amelas;
using namespace amelas::communication;
using namespace amelas::communication::common;
using namespace amelas::controller;


using zmqutils::common::CommandType;
using zmqutils::common::CommandReply;
using zmqutils::common::ServerCommand;
using zmqutils::utils::BinarySerializer;

void parseCommand(CommandClientBase &client, const std::string &command)
{
    zmqutils::common::ClientResult client_result = ClientResult::COMMAND_OK;

    char *command_str = new char[command.size()];
    std::copy(command.begin(), command.end(), command_str);

    char *token = std::strtok(command_str, " ");

    if (token)
    {
        CommandType command_id;

        try
        {
            command_id = static_cast<CommandType>(std::stoi(token));
        }
        catch (...)
        {
            std::cerr << "Failed at sending command." << std::endl;
            delete[] command_str;
            return;
        }

        RequestData command_msg(static_cast<ServerCommand>(command_id));

        bool valid = true;

        if (command_id == static_cast<CommandType>(ServerCommand::REQ_CONNECT))
        {
            std::cout << "Sending connect message" << std::endl;
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_DISCONNECT))
        {
            std::cout << "Sending disconnect message" << std::endl;
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_ALIVE))
        {
            std::cout << "Sending keepalive command." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_DATETIME))
        {
            std::cout << "Get datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_SET_DATETIME))
        {
            std::cout << "Set datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
        {
            std::cout << "Sending get home position command." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_SET_HOME_POSITION))
        {
            std::cout << "Sending set home position command." << std::endl;

            bool valid_params = true;
            double az = 0., el = 0.;
            char *param_token = std::strtok(nullptr, " ");

            try
            {
                az = std::stod(param_token);
            }
            catch (...)
            {
                std::cerr << "Bad parameter azimuth issued.";
                valid_params = false;
            }

            if (valid_params)
            {
                param_token = std::strtok(nullptr, " ");

                try
                {
                    el = std::stod(param_token);
                }
                catch (...)
                {
                    std::cerr << "Bad parameter elevation issued.";
                    valid_params = false;
                }
            }

            if (valid_params)
            {
                std::cout<<"Sending: " << az <<" "<<el<<std::endl;

                AltAzPos pos(az, el);

                BinarySerializer serializer;


                serializer.write(pos);

                AltAzPos* posi;



                std::cout<<serializer.toJsonString();

                command_msg.params_size = BinarySerializer::fastSerialization(command_msg.params, pos);

                std::cout<<"Sending size: " << command_msg.params_size <<std::endl;


                std::cout<<std::endl;
                std::cout<<std::endl;
                std::cout<<std::endl;
                std::cout<<std::endl;
                std::cout<<std::endl;


            }
            else
            {
                std::cout<<"Sending invalid command: "<<std::endl;
                command_msg.params_size = BinarySerializer::fastSerialization(command_msg.params, az);

                valid_params = true;
            }

            valid = valid_params;

        }
        else
        {
            valid = false;
        }

        // TODO MOVE ALL OF THIS TO A SUBCLASS IN A PURE VIRTUAL. THE FUNCTION WILL RETURN ClientResult
        // TODO THE ERROR CONTROL MUST BE IN THE BASE CLIENT. THE SUBCLASS MUST CONTROL THE OUTPUT DATA AND CUSTOM ERRORS ONLY.
        // TODO DISABLE SEND WITH THIS WAY THE RESERVED COMMANDS.
        // TODO CREATE doConnect, doDisconnect, checkServerAlive
        // TODO CREATE IN THE CLIENT THE INTERNAL CALLBACKS LIKE THE SERVER.
        // TODO MOVE THE PROCESSING OF EACH COMPLEX RESPONSE TO A FUNCTION.

        if (valid)
        {
            // TODO MOVE ALL
            CommandReply reply;

            if(command_msg.command == ServerCommand::REQ_CONNECT)
            {
                client_result = client.doConnect();

                if (client_result == ClientResult::CLIENT_STOPPED)
                    return;


            }
            else
                client_result = client.sendCommand(command_msg, reply);

            std::cerr << "Client Result: " << static_cast<int>(client_result)<<std::endl;

            if (client_result != ClientResult::COMMAND_OK)
            {
            }
            else
            {
                constexpr std::size_t res_sz = sizeof(amelas::controller::ControllerError);
                constexpr std::size_t double_sz = sizeof(double);

                std::cout<<"Server result: "<<static_cast<int>(reply.result)<<std::endl;

                if(reply.result != ServerResult::COMMAND_OK)
                {
                    delete[] command_str;
                    return;
                }

                // Get the controller result.
                // TODO ERROR CONTROL

                if(command_id > static_cast<CommandType>(ServerCommand::END_BASE_COMMANDS))
                {
                    ControllerError error;

                    BinarySerializer ser(reply.params.get(), reply.params_size);
                    std::cout<<ser.toJsonString()<<std::endl;

                    ser.read(error);

                    std::cout<<"Controller error: "<<static_cast<int>(error)<<std::endl;
                }

                if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
                {
                    if (reply.params_size == (res_sz + 2*double_sz))
                    {
                        ControllerError error;   // Trash
                        double az;
                        double el;

                        // Deserialize the parameters.
                        BinarySerializer::fastDeserialization(reply.params.get(), reply.params_size, error, az, el);

                        // Generate the struct.
                        std::cout<<"Az: "<<az<<std::endl;
                        std::cout<<"El: "<<el<<std::endl;
                    }
                    else
                    {
                        std::cout<<"BAD PARAMS"<<std::endl;
                        // RETURN BAD PARAMS
                        //result = ClientResult::
                    }
                }
            }
        }
        else
        {
            std::cerr << "Command is not implemented or valid" << std::endl;
        }

    }
    else
    {
        std::cerr << "Not a valid command" << std::endl;
    }


    delete[] command_str;
}


int main(int, char**)
{
    // Configure the console.
    zmqutils::internal_helpers::ConsoleConfig cmd_config(true, false, false);

    // Configuration variables.
    int port = 9999;
    std::string ip = "127.0.0.1";

    std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);

    AmelasClient client(endpoint, "AMELAS EXAMPLE CLIENT");

    // Set the exit callback to the console handler for safety.
    zmqutils::internal_helpers::ConsoleConfig::setExitCallback(
        [&client](){client.stopClient();});

    client.startClient();

    //client.startAutoAlive();
    std::string command;

    // Infinite loop for test.
    while(!zmqutils::internal_helpers::ConsoleConfig::gCloseFlag)
    {
        // Get the command and parameters.
        std::cout<<"--------------------------------------------"<<std::endl;
        std::cout<<"-- Commands --"<<std::endl;
        std::cout<<"- REQ_CONNECT:    0"<<std::endl;
        std::cout<<"- REQ_DISCONNECT: 1"<<std::endl;
        std::cout<<"- REQ_ALIVE:      2"<<std::endl;
        std::cout<<"- CUSTOM:         cmd param1 param2 ..."<<std::endl;
        std::cout<<"--------------------------------------------"<<std::endl;
        std::cout<<"Write a command: ";
        std::getline(std::cin, command);

        // Check for exit.
        if(command == "exit")
        {
            // Manual stop.
            std::cout << "Stopping the client..." << std::endl;
            client.stopClient();
            break;
        }

        // Break if we want to close the example program.
        if(zmqutils::internal_helpers::ConsoleConfig::gCloseFlag || std::cin.eof())
        {
            std::cout << std::endl;
            std::cout << "Stopping the client..." << std::endl;
            break;
        }

        // Parse the command.
        parseCommand(client, command);
    }

    // Wait for closing.
    // Neccesary due to the command handler for this example works in other thread.
    client.waitForClose();

    // Final log.
    std::cout << "Client stoped. All ok!!" << std::endl;

    // Restore the console.
    cmd_config.restoreConsole();

	return 0;
}
