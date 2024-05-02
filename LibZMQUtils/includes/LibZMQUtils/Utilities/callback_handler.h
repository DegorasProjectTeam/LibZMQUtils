/***********************************************************************************************************************
 *   LibZMQUtils (ZMQ Utilitites Library): A libre library with ZMQ related useful utilities.                          *
 *                                                                                                                     *
 *   Copyright (C) 2023 Degoras Project Team                                                                           *
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
 * @file callback_handler.h
 * @brief This file contains the declaration and template implementation of the `CallbackHandler` class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <cstdint>
#include <functional>
#include <map>
#include <any>
#include <mutex>
#include <stdexcept>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

/**
 * @class CallbackHandler
 *
 * @brief A class that manages generic callbacks functions mapped with unique identifiers.
 *
 * This class manages and invokes generic callbacks functions that have been registered with it. Callbacks are mapped
 * to identifiers (CallbackId) for reference. Only one function can be register with a certain ideintifier. The class
 * also is designed to be thread-safe.
 *
 * This class can be used independently or subclassed with other classes such as servers or clients of this library.
 *
 * @warning Remember that the function invokeCallback can throw exceptions.
 *
 */
class LIBZMQUTILS_EXPORT CallbackHandler
{
public:

    // -----------------------------------------------------------------------------------------------------------------
    using CallbackId = std::uint64_t;     ///< Alias for the callback identifiers.
    // -----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Default constructor.
     */
    CallbackHandler() = default;

    /**
     * @brief Deleted copy constructor.
     */
    CallbackHandler(const CallbackHandler&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    CallbackHandler& operator=(const CallbackHandler&) = delete;

    /**
     * @brief Deleted move constructor.
     */
    CallbackHandler(CallbackHandler&&) = delete;

    /**
     * @brief Deleted move assignment operator.
     */
    CallbackHandler& operator=(CallbackHandler&&) = delete;

    /**
     * @brief Default destructor.
     */
    ~CallbackHandler() = default;

    /**
     * @brief Register a member function as a callback replacing the old one.
     * @tparam ClassT - Type of the class that owns the member function.
     * @tparam RetT - Return type of the member function.
     * @tparam Args - Arguments of the member function.
     * @param id - The id to be assigned to the callback.
     * @param object - Pointer to the object whose member function is to be registered.
     * @param callback - Member function to be registered as a callback.
     */
    template<typename Id, typename ClassT = void, typename RetT = void, typename... Args>
    void registerCallback(Id id, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        std::lock_guard<std::mutex> lock(this->mtx_);
        callback_map_[static_cast<CallbackId>(id)] = CallbackHandler::makeCallback(object, callback);
    }

    /**
     * @brief Remove a callback using its id.
     * @param id - The id of the callback to be removed.
     */
    void removeCallback(CallbackId id);

    /**
     * @brief Check if a callback with the given id exists.
     * @param id - The id of the callback to be checked.
     * @return bool - true if callback exists, false otherwise.
     */
    bool hasCallback(CallbackId id) const;

    /**
     * @brief Remove all registered callbacks.
     */
    void clearCallbacks();

    /**
     * @brief Invoke a registered callback with the given id.
     * @tparam CallbackType - The type of the callback to be invoked.
     * @tparam RetT - Return type of the callback.
     * @tparam Args - Arguments of the callback.
     * @param id - The id of the callback to be invoked.
     * @param args - Arguments to be passed to the callback.
     * @return RetT - The return value of the callback.
     * @throw std::invalid_argument - if the callback with the given id does not exist or has incorrect type.
     * @throw std::runtime_error - if the callback is null and other exceptions.
     */
    template <typename CallbackType, typename RetT, typename... Args>
    RetT invokeCallback(CallbackId id, Args&&... args)
    {
        // Safe mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);
        // Try the any cast.
        try
        {
            auto& any_obj = this->callback_map_.at(id);
            auto callback = std::any_cast<CallbackType>(&any_obj);
            if(callback)
            {
                // Call to the callback function.
                return (*callback)(std::forward<Args>(args)...);
            }
        } catch (const std::out_of_range&)
        {throw std::invalid_argument("CallbackHandler - Callback with the provided id does not exist.");
        }catch (const std::bad_any_cast&)
        {throw std::invalid_argument("CallbackHandler - Incorrect callback type.");
        } catch (...)
        {throw std::runtime_error("CallbackHandler - Other exception.");}

        // Throw an exception when the callback is null.
        throw std::runtime_error("CallbackHandler - Callback is null.");
    }

private:

    /**
     * @brief Helper function to create a std::function from a member function.
     * @tparam ClassT - Type of the class that owns the member function.
     * @tparam ReturnT - Return type of the member function.
     * @tparam Args - Arguments of the member function.
     * @param object - Pointer to the object whose member function is to be registered.
     * @param memberFunction - Member function to be registered as a callback.
     * @return std::function - A std::function encapsulating the member function.
     */
    template<typename ClassT, typename ReturnT, typename... Args>
    static std::function<ReturnT(Args...)> makeCallback(ClassT* object, ReturnT(ClassT::*memberFunction)(Args...))
    {
        return [object, memberFunction](Args... args) -> ReturnT
        {
            return (object->*memberFunction)(std::forward<Args>(args)...);
        };
    }

    // Private members.
    std::map<CallbackId, std::any> callback_map_;    ///< Map storing callbacks against their ids.
    mutable std::mutex mtx_;                         ///< Safety mutex.
};


}} // END NAMESPACES.
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
//#include "LibZMQUtils/Utilities/callback_handler.tpp"
// =====================================================================================================================
