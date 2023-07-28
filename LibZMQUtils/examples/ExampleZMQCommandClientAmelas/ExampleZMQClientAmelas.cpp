

#include <iostream>
#include <cstring>

#include <LibZMQUtils/Utils>

#include "AmelasExampleController/common.h"
#include "AmelasExampleServer/common.h"
#include "AmelasExampleClient/amelas_client.h"


// TODO Remove
using namespace zmqutils;
using namespace amelas;
using namespace amelas::cltsrv;
using namespace amelas::cltsrv::common;
using namespace amelas::controller;


using zmqutils::common::CommandType;
using zmqutils::common::ServerCommand;

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

        RequestData command_msg(command_id);

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

                command_msg.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[16]);
                command_msg.params_size = 16;

                zmqutils::utils::binarySerializeDeserialize(&az, 8, command_msg.params.get());
                zmqutils::utils::binarySerializeDeserialize(&el, 8, command_msg.params.get() + 8);
            }
            else
            {
                std::cout<<"Sending invalid command: "<<std::endl;
                double az = 0;
                command_msg.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[16]);
                command_msg.params_size = 8;
                zmqutils::utils::binarySerializeDeserialize(&az, 8, command_msg.params.get());
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
            ClientResult result = ClientResult::COMMAND_OK;
            CommandReply reply;

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

                    zmqutils::utils::BinarySerializer ser(reply.params.get(), reply.params_size);
                    std::cout<<ser.toString()<<std::endl;

                    ser.read(error);

                    std::cout<<"Controller error: "<<static_cast<int>(error)<<std::endl;
                }

                if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
                {
                    if (reply.params_size == (res_sz + 2*double_sz))
                    {
                        double az;
                        double el;

                        // Deserialize the parameters.
                        zmqutils::utils::binarySerializeDeserialize(reply.params.get() + res_sz, double_sz, &az);
                        zmqutils::utils::binarySerializeDeserialize(reply.params.get() + res_sz + double_sz, double_sz, &el);

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


int main(int argc, char**argv)
{

    int port = 9999;
    std::string ip = "127.0.0.1";

    if (argc == 2)
    {
        ip = argv[1];
    }
    if (argc == 3)
    {
        ip = argv[1];
        try
        {
            port = std::stoi(argv[2]);
        }  catch (...)
        {
            std::cerr << "Not recognized port in input: " << argv[2] << std::endl;
            return -1;
        }

    }
    else if (argc > 3)
    {
        std::cout << "Usage: ZMQClient [ip] [port]" << std::endl;
        return 0;
    }

    std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);
    AmelasClient client(endpoint);
    client.startClient("Ethernet");
    //client.setClientHostIP("");
    std::cout << "Connecting to endpoint: " <<  endpoint << std::endl;
    //client.startAutoAlive();
    std::string command;

    while(true)
    {
        std::cout<<"Write a command: ";
        std::getline(std::cin, command);

        if (command == "exit")
            break;

        parseCommand(client, command);
    }

    std::cout << "Requested client to stop. Bye." << std::endl;

    client.stopClient();

	
	return 0;
}
