

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
#include "amelas_example_server.h"
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

// Main function.
//
// In the main we will create an AmelasController and an AmelasServer that will
// work together thanks to the callbacks. For safe finish, press ctrl-c.
//
int main(int argc, char**argv)
{
    // Set up the Windows Console Control Handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    // Configuration variables.
    unsigned port = 9999;
    bool client_status_check = false;

    // Get the port.
    if (argc == 2)
    {
        try{port = std::stoi(argv[1]);
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

    // Instantiate the Amelas controller.
    AmelasExampleController amelas_controller;

    // Instantiate the server.
    AmelasExampleServer amelas_server(port);

    // Disable or enables the client status checking.
    amelas_server.setClientStatusCheck(client_status_check);

    // Set the controller callbacks in the server.
    // Home position.
    amelas_server.setCallback(AmelasServerCommand::REQ_SET_HOME_POSITION,
                              &amelas_controller, &AmelasExampleController::setHomePosition);

    // Start the server.
    amelas_server.startServer();

    // Use the condition variable as an infinite loop until ctrl-c.
    std::unique_lock<std::mutex> lock(gMtx);
    gExitCv.wait(lock, [] { return gSignInterrupt == 1; });

    // Stop the server and wait the future.
    amelas_server.stopServer();

    // Final log.
    std::cout << "Server stoped. Press Enter to exit!" << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
