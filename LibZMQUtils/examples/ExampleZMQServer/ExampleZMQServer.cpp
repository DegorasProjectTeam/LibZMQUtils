

// C++ INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <limits>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "custom_command_server.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

// Global variables for safety ending.
volatile sig_atomic_t gSignInterrupt = 0;
std::condition_variable gExitCv;
std::mutex gMtx;

// Signal handler for safety ending.
#ifdef _WIN32
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    std::lock_guard<std::mutex> lock(gMtx);
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
    {
        if (!gSignInterrupt)
        {
            gSignInterrupt = 1;
            gExitCv.notify_all();
        }
        return TRUE;
    }
    return FALSE;
}
#else
// TODO
#endif

// ---------------------------------------------------------------------------------------------------------------------



// External connect callback.
void connectCallback(const void *, size_t , void *&data_out, size_t &out_size)
{

}

// External disconnect callback.
void disconnectCallback(const void *, size_t , void *&data_out, size_t &out_size)
{

}

// External alive callback.
void aliveCallback(const void *, size_t , void *&data_out, size_t &out_size)
{
//    std::cout << "Client request for alive. I AM alive." << std::endl;
//    out_size = sizeof(common::CommandReqId) + sizeof(CommandServerBase::CommandResult);
//    auto *data_out_bytes = new std::uint8_t[out_size];
//    auto result = CommandServerBase::CommandResult::COMMAND_OK;
//    // To serialize parameters insert them byte by byte consecutively
//    // First data is command id at position [0, sizeof(CommandId))
//    utils::binarySerializeDeserialize(&CommandServerBase::kAliveCommand,
//                                      sizeof(common::CommandReqId), data_out_bytes);
//    // Second data is command result at position [sizeof(CommandId), sizeof(CommandId) + sizeof(CommandError))
//    utils::binarySerializeDeserialize(&result, sizeof(CommandServerBase::CommandResult),
//                                      data_out_bytes + sizeof(common::CommandReqId));
//    data_out = data_out_bytes;
}

int main(int argc, char**argv)
{
    // Set up the Windows Console Control Handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    unsigned port = 9999;

    if (argc == 2)
    {
        try
        {
           port = std::stoi(argv[1]);
        }  catch (...)
        {
            std::cerr << "Not recognized port in input: " << argv[1] << std::endl;
            return -1;
        }

    }
    else if (argc > 2)
    {
        std::cout << "Usage: ZMQServer [port]" << std::endl;
        return 0;
    }


    CustomCommandServer server("*", port);
    //server.setDeadClientCallback([]{std::cout << "Dead client timeout reached." <<std::endl;});
    //server.setCommandCallback(ZMQServer::kConnectCommand, connectCallback);
    //server.setCommandCallback(CommandServerBase::kDisconnectCommand, disconnectCallback);
    //server.setCommandCallback(CommandServerBase::kAliveCommand, aliveCallback);

    // Start the server.
    server.startServer();

    // Log.
    std::cout << "Server is listening at port: " << port << std::endl;


    std::unique_lock<std::mutex> lock(gMtx);
    gExitCv.wait(lock, [] { return gSignInterrupt == 1; });

    std::cout << "Stopping the server..." << std::endl;

    // Stop the server and wait the future.
    server.stopServer();


    // Final log.
    std::cout << "Server stoped. Press Enter to exit!" << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    // Return.
	return 0;
}
