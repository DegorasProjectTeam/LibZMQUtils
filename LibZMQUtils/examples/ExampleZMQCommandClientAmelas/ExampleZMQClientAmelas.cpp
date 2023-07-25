

#include <iostream>
#include <cstring>

#include <LibZMQUtils/CommandClient>



using namespace zmqutils;

// Specific subclass commands (0 to 4 are reserved for the base server).
// WARNING: In our approach, the server commands must be always in order.
enum class AmelasServerCommand : common::CommandType
{
    REQ_SET_DATETIME      = 11,
    REQ_GET_DATETIME      = 12,
    REQ_SET_HOME_POSITION = 13,
    REQ_GET_HOME_POSITION = 14,
    END_AMELAS_COMMANDS
};

// Specific subclass errors (0 to 15 are reserved for the base server).
enum class AmelasServerResult : common::CommandType
{
    INVALID_DATETIME = 16,
    INVALID_POSITION = 17
};

void parseCommand(CommandClientBase &client, const std::string &command)
{
    void *data_out = nullptr;
    size_t out_size_bytes = 0;
    int send_result = 0;

    char *command_str = new char[command.size()];
    std::copy(command.begin(), command.end(), command_str);

    char *token = std::strtok(command_str, " ");

    if (token)
    {
        common::CommandType command_id;

        try
        {
            command_id = static_cast<common::CommandType>(std::stoi(token));
        }
        catch (...)
        {
            std::cerr << "Failed at sending command." << std::endl;
            delete[] command_str;
            return;
        }

        CommandData command_msg(command_id);
        bool valid = true;

        if (command_id == static_cast<common::CommandType>(common::BaseServerCommand::REQ_CONNECT))
        {
            std::cout << "Sending connect message" << std::endl;

            //client.startAutoAlive();
        }
        else if (command_id == static_cast<common::CommandType>(common::BaseServerCommand::REQ_DISCONNECT))
        {
            std::cout << "Sending disconnect message" << std::endl;

            client.stopAutoAlive();
        }
        else if (command_id == static_cast<common::CommandType>(common::BaseServerCommand::REQ_ALIVE))
        {
            std::cout << "Sending keepalive command." << std::endl;
        }
        else if (command_id == static_cast<common::CommandType>(AmelasServerCommand::REQ_GET_DATETIME))
        {
            std::cout << "Get datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<common::CommandType>(AmelasServerCommand::REQ_SET_DATETIME))
        {
            std::cout << "Set datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<common::CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
        {
            std::cout << "Sending get home position command." << std::endl;
        }
        else if (command_id == static_cast<common::CommandType>(AmelasServerCommand::REQ_SET_HOME_POSITION))
        {
            std::cout << "Sending get home position command." << std::endl;

            bool valid_params = true;
            double az, el;
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

            valid = valid_params;

            std::cout<<"Sending: " << az <<" "<<el<<std::endl;

            command_msg.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[16]);
            command_msg.params_size = 16;

            zmqutils::utils::binarySerializeDeserialize(&az, 8, command_msg.params.get());
            zmqutils::utils::binarySerializeDeserialize(&el, 8, command_msg.params.get() + 8);

        }
        else
        {
            valid = false;
        }

        if (valid)
        {
            send_result = client.sendCommand(command_msg, data_out, out_size_bytes);

            if (send_result != 0)
            {
                std::cerr << "Command sending failed with code: " << send_result << std::endl;
                // Restart client if sending fails
                client.resetClient();
            }

            else if (out_size_bytes >= sizeof(CommandClientBase::CommandError))
            {
                CommandClientBase::CommandError error_response;

                auto *data_bytes = static_cast<std::uint8_t*>(data_out);
                zmqutils::utils::binarySerializeDeserialize(
                            data_bytes, sizeof(CommandClientBase::CommandError), &error_response);
                std::cout << "Response code from server: "<< static_cast<std::uint32_t>(error_response) << std::endl;

                if (command_id == static_cast<common::CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
                {
                    double az, el;
                    if (out_size_bytes == sizeof(CommandClientBase::CommandError) + 16)
                    {
                        zmqutils::utils::binarySerializeDeserialize(
                                    data_bytes + sizeof(CommandClientBase::CommandError), 8, &az);

                        zmqutils::utils::binarySerializeDeserialize(
                                    data_bytes + sizeof(CommandClientBase::CommandError) + 8, 8, &el);

                        std::cout << "Get home position command result is (az,el): " << az << ", " << el << std::endl;
                    }
                    else
                        std::cerr << "Get home position command answer is incorrect." << std::endl;

                }

                delete[] data_bytes;
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
    CommandClientBase client(endpoint);
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
