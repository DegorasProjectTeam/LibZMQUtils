/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibZMQUtils.                                                                                 *
 *                                                                                                                     *
 *   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
 *   as soon they will be approved by the European Commission (IDABC).                                                 *
 *                                                                                                                     *
 *   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
 *   published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
 *                                                                                                                     *
 *   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
 *   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
 *   language governing permissions and limitations and more details.                                                  *
 *                                                                                                                     *
 *   You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
 *   along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
 **********************************************************************************************************************/

/** ********************************************************************************************************************
 * @file clbk_command_server_base.h
 * @brief This file contains the declaration of the ClbkCommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/CommandServerClient/command_server/command_server_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/InternalHelpers/tuple_helpers.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

/**
 * @brief The ClbkCommandServerBase class implements a CommandServer that includes callback handling for each command.
 */
class LIBZMQUTILS_EXPORT ClbkCommandServerBase : public CommandServerBase,
                                                 private utils::CallbackHandler
{
public:

    /**
     * @brief ClbkCommandServerBase default constructor.
     */
    ClbkCommandServerBase(unsigned server_port,
                          const std::string& server_iface = "*",
                          const std::string& server_name = "",
                          const std::string& server_version = "",
                          const std::string& server_info = "");

    /**
     * @brief Template function for registering a callback. This callback will be registered for a specific command.
     *
     * This template function register a callback for a specific request command. In this case, no request process
     * function will be register, so, you must program in your subclass ClbkCommandServerBase each function to process
     * each request and inside those functions, you can call to the register callbacks manually.
     *
     * If you want register the callback as well as an automaticed process function for the specific request, you
     * can use the funticon registerCallbackAndRequestProcFunc.
     *
     * @param command, the command the callback is applied to.
     * @param object, a parametric object whose method will be called.
     * @param callback, the callback method that will be called.
     */
    template<typename CmdId, typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(CmdId command, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        CallbackHandler::registerCallback(static_cast<CallbackHandler::CallbackId>(command), object, callback);
    }

    /**
     * @brief Registers a callback and an associated request processing function for a specific command.
     *
     * This function not only registers a callback for a specific command but also sets up an automated
     * process function to handle requests of that command type. The process function automatically
     * invokes the registered callback with appropriate parameters extracted from the request.
     *
     * This approach simplifies the setup process by automatically linking the command processing logic
     * with the appropriate callback, thereby reducing manual boilerplate code and potential errors.
     *
     * @tparam CallbackType The type of the callback handler, usually determining how the callback will
     *         be invoked and with what parameters.
     * @tparam InputTuple A tuple describing the types of data expected as input from the request.
     *         Used to deserialize and pass data to the callback.
     * @tparam OutputTuple A tuple describing the types of data that will be output or modified by the
     *         callback and need serialization into the reply.
     * @tparam CmdId The type of the command identifier (usually an enum or integral type).
     * @tparam ClassT The class type on which the member function callback is defined.
     * @tparam RetT The return type of the callback function. Defaults to void.
     * @tparam Args Variadic template parameters representing the types of the arguments that the
     *         callback function accepts.
     *
     * @param command The identifier for the command with which this callback and process function are
     *        associated.
     * @param object Pointer to the instance of the object on which the callback method will be called.
     * @param callback Member function pointer to the callback method that will be invoked to process
     *        the command.
     */
    template<typename CallbackType, typename InputTuple = std::tuple<>, typename OutputTuple = std::tuple<>,
             typename CmdId, typename ClassT, typename RetT = void, typename... Args>
    void registerCbAndReqProcFunc(CmdId command, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        // Register the callback.
        this->registerCallback(command, object, callback);

        // Process function lambda.
        auto lambda_proc_func = [this](const CommandRequest& request, CommandReply& reply)
        {
            this->processClbkRequest<CallbackType, RetT, InputTuple, OutputTuple>(request, reply);
        };

        // Automatic command process function registration.
        this->registerReqProcFunc(static_cast<zmqutils::reqrep::CommandType>(command), lambda_proc_func);
    }

    /**
     * @brief Remove the registered callback for a specific command.
     * @param command, the command whose callback will be erased.
     */
    void removeCallback(ServerCommand command);

    /**
     * @brief Check if there is a registered callback for a specific command.
     * @param command, the command whose callback existence will be checked.
     * @return
     */
    bool hasCallback(ServerCommand command);

    /**
     * @brief Virtual destructor.
     */
    virtual ~ClbkCommandServerBase() override;

protected:

    /**
     * @brief Processes a callback request based on the command type and bytes encapsulated in the request.
     *
     * This function processes different types of callback requests by handling input and output parameters
     * differently depending on their types, which are specified as tuple template parameters.
     * It supports several scenarios:
     * - Only input parameters are provided.
     * - Only output parameters are provided.
     * - Both input and output parameters are provided.
     * - Neither input nor output parameters are provided.
     *
     * The function deserializes input bytes from the request, invokes the appropriate callback based on the
     * template parameters, and serializes the output back into the reply.
     *
     * @tparam CallbackType The type of the callback function to be invoked.
     * @tparam RetT The return type of the callback function.
     * @tparam InputTuple A tuple containing types of the input parameters.
     * @tparam OutputTuple A tuple containing types of the output parameters.
     * @param request A reference to the CommandRequest object containing input bytes and command details.
     * @param reply A reference to the CommandReply object to store the results of the callback invocation.
     */
    template<typename CallbackType, typename RetT, typename InputTuple, typename OutputTuple>
    void processClbkRequest(const zmqutils::reqrep::CommandRequest& request,
                            zmqutils::reqrep::CommandReply& reply)
    {
        // Prepare the input and output parameters
        InputTuple inputs;
        OutputTuple outputs;

        // If there are inputs, deserialize them
        if constexpr (std::tuple_size_v<InputTuple> > 0)
        {
            if (request.data.isEmpty())
            {
                reply.result = zmqutils::reqrep::OperationResult::EMPTY_PARAMS;
                return;
            }

            // Deserialize the inputs.
            try
            {
                zmqutils::serializer::BinarySerializer::fastDeserialization(request.data.bytes.get(),
                                                                            request.data.size,
                                                                            inputs);
            }
            catch(...)
            {
                reply.result = OperationResult::BAD_PARAMETERS;
                return;
            }
        }

        // Concat inputs and outputs into a single tuple
        auto args = std::tuple_cat(std::move(inputs), std::move(outputs));

        // If return type is void, discard return from callback and send only outputs if necessary.
        if constexpr (std::is_void_v<RetT>)
        {
            // Invoke the callback with parameters
            std::apply([this, &request, &reply](auto&&... args)
            {
                return this->invokeCallback<CallbackType, RetT>(request,
                                                                reply,
                                                                std::forward<decltype(args)>(args)...);
            }, args);

            // If there are output parameters, serialize them into the reply.
            if constexpr (std::tuple_size_v<OutputTuple> > 0)
            {
                // Get the output parameters from the parameters tuple.
                internal_helpers::tuple::tuple_split(std::move(args), inputs, outputs);

                // Serialize the output parameters.
                reply.data.size = zmqutils::serializer::BinarySerializer::fastSerialization(reply.data.bytes, outputs);
            }
        }
        // If there are return type at callback, send it before output parameters.
        else
        {
            // Invoke the callback with parameters and handle return value
            auto ret = std::apply([this, &request, &reply](auto&&... args)
            {
                return this->invokeCallback<CallbackType, RetT>(request, reply, std::forward<decltype(args)>(args)...);
            }, args);

            // Serialize the return value.
            zmqutils::serializer::BinarySerializer serializer;
            serializer.write(ret.value());

            // If there are output parameters, serialize them.
            if constexpr (std::tuple_size_v<OutputTuple> > 0)
            {
                // Get the output parameters from the parameters tuple
                internal_helpers::tuple::tuple_split(std::move(args), inputs, outputs);

                // Serialize the output parameters.
                serializer.write(outputs);
            }

            // Serialize the data and move the container.
            reply.data.size = serializer.moveUnique(reply.data.bytes);
        }
    }

    /**
     * @brief Parametric method for invoking a registered callback. If no callback is registered, an error is returned.
     * @param msg, the received message.
     * @param args, the args passed to the callback.
     * @return the result of the callback inovocation.
     */
    template <typename CallbackType, typename RetT,  typename... Args>
    std::conditional_t<std::is_void_v<RetT>, void, std::optional<RetT>>
    invokeCallback(const CommandRequest& request, CommandReply& reply, Args&&... args)
    {
        // Get the command and prepare the return.
        ServerCommand cmd = static_cast<ServerCommand>(request.command);

        // Check the callback.
        if(!this->hasCallback(cmd))
        {
            reply.result = OperationResult::EMPTY_EXT_CALLBACK;
            return RetT();
        }

        //Invoke the callback.
        try
        {
            if constexpr (std::is_void_v<RetT>)
            {
                CallbackHandler::invokeCallback<CallbackType, RetT>(
                    static_cast<CallbackHandler::CallbackId>(cmd), std::forward<Args>(args)...);
                return;
            }
            else
            {
                return CallbackHandler::invokeCallback<CallbackType, RetT>(
                    static_cast<CallbackHandler::CallbackId>(cmd), std::forward<Args>(args)...);
            }
        }
        catch(...)
        {
            reply.result = OperationResult::INVALID_EXT_CALLBACK;
            return RetT();
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::registerCallback;
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;
};

}} // END NAMESPACES.
// =====================================================================================================================
