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

// C++ INCLUDES
// =====================================================================================================================

#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <omp.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Publisher>
#include <LibZMQUtils/Modules/Subscriber>
#include <LibZMQUtils/Testing/UnitTest>
// =====================================================================================================================

// Configuration variables.
static const unsigned port = 9999;
static const std::string ip = "127.0.0.1";
static const std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);

class TestSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
{
private:
    // SubscriberBase interface
    void onSubscriberStart() override {}
    void onSubscriberStop() override {}
    void onSubscriberError(const zmq::error_t &, const std::string &) override {}
};


// Basic tests.
M_DECLARE_UNIT_TEST(PublisherSubscriber, PublishSubscribe)
M_DECLARE_UNIT_TEST(PublisherSubscriber, PublishMultithread)


// Implementations.

M_DEFINE_UNIT_TEST(PublisherSubscriber, PublishSubscribe)
{
    class SubscriberCallbackHandler
    {
    private:
        std::promise<std::string> promise;

    public:
        SubscriberCallbackHandler() : future(promise.get_future()) {}

        void handleMsg(const std::string &msg)
        {
            this->promise.set_value(msg);
        }


        std::future<std::string> future;
    };

    std::cout << "Start test..." << std::endl;

    SubscriberCallbackHandler handler;
    const std::string test_string = "HOLA MUNDO";

    // Publisher.
    zmqutils::pubsub::PublisherBase publisher(endpoint, "Test publisher");

    // Start the publisher.
    bool started = publisher.startPublisher();

    if(!started)
    {
        std::cout << "Failed at start publisher" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Subscriber
    TestSubscriber subscriber;

    subscriber.subscribe(endpoint);
    subscriber.addTopicFilter("Test");

    // Start the subscriber.
    started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!started)
    {
        std::cout << "Failed at start subscriber" << std::endl;
        M_FORCE_FAIL()
        return;
    }


    subscriber.registerCallbackAndRequestProcFunc<std::function<void(const std::string&)>>(
        "Test", &handler, &SubscriberCallbackHandler::handleMsg);

    publisher.sendMsg("Test", test_string);

    std::string received_string = handler.future.get();

    M_EXPECTED_EQ(received_string, test_string)

    std::cout << "End test..." << std::endl;

}


M_DEFINE_UNIT_TEST(PublisherSubscriber, PublishMultithread)
{
    const std::string test_string = "HOLA MUNDO";
    const int messages_to_receive = 100000;

    class SubscriberCallbackHandler
    {
    private:
        std::promise<bool> promise;
        std::atomic_int messages;

        const std::string test_msg;
        const int messages_to_receive;

    public:
        SubscriberCallbackHandler(const std::string &test_string, int messages_to_receive) :
            messages(0),
            test_msg(test_string),
            messages_to_receive(messages_to_receive),
            future(promise.get_future()) {}

        void handleMsg(const std::string &msg)
        {
            if (msg == this->test_msg)
            {
                messages++;
                if (messages == this->messages_to_receive)
                    this->promise.set_value(true);
            }
            else
            {
                std::cout << "Invalid message received" << std::endl;
                this->promise.set_value(false);
            }
        }


        std::future<bool> future;
    };

    std::cout << "Start test..." << std::endl;

    SubscriberCallbackHandler handler(test_string, messages_to_receive);

    // Publisher.
    zmqutils::pubsub::PublisherBase publisher(endpoint, "Test publisher");

    // Start the publisher.
    bool publisher_started = publisher.startPublisher();

    if(!publisher_started)
    {
        std::cout << "Failed at start publisher " << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Subscriber
    TestSubscriber subscriber;

    subscriber.subscribe(endpoint);
    subscriber.addTopicFilter("Test");

    // Start the subscriber.
    bool subscriber_started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!subscriber_started)
    {
        std::cout << "Failed at start subscriber" << std::endl;
        M_FORCE_FAIL()
        return;
    }


    subscriber.registerCallbackAndRequestProcFunc<std::function<void(const std::string&)>>(
        "Test", &handler, &SubscriberCallbackHandler::handleMsg);


    std::vector<std::future<void>> publisher_futures;


    for (int i = 0; i < messages_to_receive; i++)
    {
        publisher_futures.push_back(std::async(std::launch::async, [&publisher, &test_string]
        {
            publisher.sendMsg("Test", test_string);
        }));
    }

    if (!this->result_)
        return;

    bool ok = handler.future.get();

    M_EXPECTED_EQ(ok, true)

    std::cout << "End test..." << std::endl;

}



int main()
{
    // Start of the session.
    M_START_UNIT_TEST_SESSION("LibZMQUtils PublisherSubscriber Session")

    // Register the tests.
    M_REGISTER_UNIT_TEST(PublisherSubscriber, PublishSubscribe)
    M_REGISTER_UNIT_TEST(PublisherSubscriber, PublishMultithread)


    // Run the unit tests.
    M_RUN_UNIT_TESTS()
}
