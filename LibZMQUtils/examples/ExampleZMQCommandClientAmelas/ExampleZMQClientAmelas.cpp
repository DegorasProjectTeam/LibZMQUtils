

#include <iostream>
#include <cstring>

#include <LibZMQUtils/CommandClient>



using namespace zmqutils;

void parseCommand(CommandClientBase &client, const std::string &command)
{
    void *data_out = nullptr;
    size_t out_size_bytes = 0;
    common::CmdRequestId command_response;
    int send_result = 0;

    char *command_str = new char[command.size()];
    std::copy(command.begin(), command.end(), command_str);

    char *token = std::strtok(command_str, " ");

    if (token)
    {
        common::BaseServerCommand command_id;

        try
        {
            command_id = static_cast<common::BaseServerCommand>(std::stoi(token));
        }
        catch (...)
        {
            std::cerr << "Failed at sending command." << std::endl;
            delete[] command_str;
            return;
        }

        if (command_id == common::BaseServerCommand::REQ_CONNECT)
        {
            std::cout << "Sending connect message" << std::endl;

            CommandData command_msg(command_id);

            send_result = client.sendCommand(command_msg, data_out, out_size_bytes);

            //client.startAutoAlive();
        }
        else
        {

            std::cout << "Sending generic command..." << std::endl;

            CommandData command_msg(command_id);

            send_result = client.sendCommand(command_msg, data_out, out_size_bytes);

        }

        if (send_result != 0)
        {
            std::cerr << "Command sending failed with code: " << send_result << std::endl;
            // Restart client if sending fails
            client.resetClient();
        }
        
        else if (out_size_bytes >= sizeof(common::CmdRequestId) + sizeof(CommandClientBase::CommandError))
        {
            CommandClientBase::CommandError error_response;

            auto *data_bytes = static_cast<std::uint8_t*>(data_out);
            CommandClientBase::binarySerializeDeserialize(data_bytes, sizeof(CommandClientBase::CommandError), &error_response);
            std::cout << "Response from server: "<< static_cast<std::uint32_t>(error_response) << std::endl;

            delete[] data_bytes;
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
