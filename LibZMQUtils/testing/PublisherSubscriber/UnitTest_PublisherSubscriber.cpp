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

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <vector>
#include <omp.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/PublisherSubscriber>
#include <LibZMQUtils/Modules/Testing>
// =====================================================================================================================

// Basic tests.
M_DECLARE_UNIT_TEST(PublisherSubscriber, BasicPublishSubscribe)
M_DECLARE_UNIT_TEST(PublisherSubscriber, RegisterCbAndReqProcFunc)

// Advanced tests.
M_DECLARE_UNIT_TEST(PublisherSubscriber, MultithreadPublishSubscribe)


// Implementations.

M_DEFINE_UNIT_TEST(PublisherSubscriber, BasicPublishSubscribe)
{
    class TestSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
    {
    private:

        using zmqutils::pubsub::ClbkSubscriberBase::ClbkSubscriberBase;

        inline void onSubscriberStart() override {}

        inline void onSubscriberStop() override {}

        inline void onSubscriberError(const zmq::error_t &, const std::string &) override {}
    };

    class SubscriberCallbackHandler
    {
    private:

        std::promise<std::string> promise_;

    public:

        inline SubscriberCallbackHandler() : future_(promise_.get_future()) {}

        inline void handleMsg(const std::string &msg)
        {
            this->promise_.set_value(msg);
        }

        std::future<std::string> future_;
    };

    // Publisher configuration variables.
    unsigned publisher_port = 9999;
    std::string publisher_iface = "*";
    std::string publisher_name = "TEST PUBLISHER";
    std::string publisher_version = "1.1.1";
    std::string publisher_info = "This is the TEST publisher";

    // Subscriber configuration variables.
    std::string subscriber_name = "TEST SUBSCRIBER";
    std::string subscriber_version = "1.1.1";
    std::string subscriber_info = "This is the TEST subscriber.";
    std::string publisher_endpoint = "tcp://127.0.0.1:9999";

    // Test data.
    const std::string test_topic = "TEST_TOPIC";
    std::string test_string = "HOLA MUNDO";
    std::string received_string;

    // Instanciate the publisher.
    zmqutils::pubsub::PublisherBase publisher(publisher_port, publisher_iface, publisher_name,
                                              publisher_version, publisher_info);

    // Start the publisher.
    bool started = publisher.startPublisher();

    // Check if started.
    if(!started)
    {
        std::cout << "Publisher start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Subscriber and callback handler.
    TestSubscriber subscriber(subscriber_name, subscriber_version, subscriber_info);
    SubscriberCallbackHandler handler;

    // Configure the subscriber.
    subscriber.subscribe(publisher_endpoint);
    subscriber.addTopicFilter(test_topic);

    // Register the callback.
    subscriber.registerCbAndReqProcFunc<std::function<void(const std::string&)>>(
        test_topic, &handler, &SubscriberCallbackHandler::handleMsg);

    // Start the subscriber.
    started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!started)
    {
        std::cout << "Subscriber start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Send and get the test msg.
    publisher.enqueueMsg(test_topic, zmqutils::pubsub::MessagePriority::NormalPriority, std::move(test_string));
    handler.future_.wait();
    received_string = handler.future_.get();

    // Stop all.
    publisher.stopPublisher();
    subscriber.stopSubscriber();

    // Check results.
    M_EXPECTED_EQ(received_string, test_string)
}

M_DEFINE_UNIT_TEST(PublisherSubscriber, RegisterCbAndReqProcFunc)
{
    class TestData : public zmqutils::serializer::Serializable
    {
    public:

        inline TestData() :
            test_number_(0), test_str_("")
        {}

        inline TestData(const std::string& test_str, const double& test_number):
            test_number_(test_number), test_str_(test_str)
        {}

        inline size_t serialize(zmqutils::serializer::BinarySerializer& serializer) const final
        {
            return serializer.write(this->test_number_, this->test_str_);
        }

        inline void deserialize(zmqutils::serializer::BinarySerializer& serializer) final
        {
            serializer.read(this->test_number_, this->test_str_);
        }

        inline size_t serializedSize() const final
        {
            return Serializable::calcSizeHelper(this->test_number_, this->test_str_);
        }

        double test_number_;
        std::string test_str_;
    };

    using PublishedTestData = zmqutils::pubsub::PublishedMessageDeserialized<TestData>;

    class TestSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
    {
    private:

        using zmqutils::pubsub::ClbkSubscriberBase::ClbkSubscriberBase;

        inline void onSubscriberStart() override {}

        inline void onSubscriberStop() override {}

        inline void onSubscriberError(const zmq::error_t &, const std::string &) override {}
    };

    class SubscriberCallbackHandler
    {
    private:

        std::promise<PublishedTestData> promise_;

    public:

        inline SubscriberCallbackHandler() : future_(promise_.get_future()) {}

        inline void handlePublishedTestData(const PublishedTestData& publication)
        {
            this->promise_.set_value(publication);
        }

        std::future<PublishedTestData> future_;
    };

    // Publisher configuration variables.
    unsigned publisher_port = 9999;
    std::string publisher_iface = "*";
    std::string publisher_name = "TEST PUBLISHER";
    std::string publisher_version = "1.1.1";
    std::string publisher_info = "This is the TEST publisher";

    // Subscriber configuration variables.
    std::string subscriber_name = "TEST SUBSCRIBER";
    std::string subscriber_version = "1.1.1";
    std::string subscriber_info = "This is the TEST subscriber.";
    std::string publisher_endpoint = "tcp://127.0.0.1:9999";

    // Test data.
    const std::string test_topic = "TEST_TOPIC";
    std::string test_string = "HOLA MUNDO";
    double test_number = 3.14;
    PublishedTestData received_publication;
    std::future_status fut_status;
    using HandlePublishedTestDataFunction = std::function<void(const PublishedTestData&)>;

    // Instanciate the publisher.
    zmqutils::pubsub::PublisherBase publisher(publisher_port, publisher_iface, publisher_name,
                                              publisher_version, publisher_info);

    // Start the publisher.
    bool started = publisher.startPublisher();

    // Check if started.
    if(!started)
    {
        std::cout << "Publisher start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Subscriber and callback handler.
    TestSubscriber subscriber(subscriber_name, subscriber_version, subscriber_info);
    SubscriberCallbackHandler handler;

    // Configure the subscriber.
    subscriber.subscribe(publisher_endpoint);
    subscriber.addTopicFilter(test_topic);

    // TODO ALL TEST AND FUNCTIONALITY TO RECOVER AUTOMATICALLY THE ORIGINAL PUB DATA.
    // Register the callback.
    // subscriber.registerDeserializedCallback<TestData>(
    //       test_topic, &handler, &SubscriberCallbackHandler::handlePublishedTestData);

    // Start the subscriber.
    started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!started)
    {
        std::cout << "Subscriber start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Send and wait the msg.
    publisher.enqueueMsg(test_topic, zmqutils::pubsub::MessagePriority::NormalPriority,
                         TestData({test_string, test_number}));
    fut_status = handler.future_.wait_for(std::chrono::milliseconds(5000));

    // Check the future.
    if(fut_status != std::future_status::ready)
    {
        M_FORCE_FAIL()
        return;
    }

    // Get the future.
    received_publication = handler.future_.get();

    // Stop all.
    publisher.stopPublisher();
    subscriber.stopSubscriber();

    // Check results.
    M_EXPECTED_EQ(received_publication.data.test_str_, test_string)
    M_EXPECTED_EQ_F(received_publication.data.test_number_, test_number, 0.000001)
}

M_DEFINE_UNIT_TEST(PublisherSubscriber, MultithreadPublishSubscribe)
{
    class TestData : public zmqutils::serializer::Serializable
    {
    public:

        inline TestData() :
            n_msg_(1), test_str_("")
        {}

        inline TestData(const std::string& test_str, const unsigned& n_msg):
            n_msg_(n_msg), test_str_(test_str)
        {}

        inline size_t serialize(zmqutils::serializer::BinarySerializer& serializer) const final
        {
            return serializer.write(this->n_msg_, this->test_str_);
        }

        inline void deserialize(zmqutils::serializer::BinarySerializer& serializer) final
        {
            serializer.read(this->n_msg_, this->test_str_);
        }

        inline size_t serializedSize() const final
        {
            return Serializable::calcSizeHelper(this->n_msg_, this->test_str_);
        }

        unsigned n_msg_;
        std::string test_str_;
    };

    class TestSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
    {
    private:

        using zmqutils::pubsub::ClbkSubscriberBase::ClbkSubscriberBase;

        void onSubscriberStart() override {}

        void onSubscriberStop() override {}

        void onSubscriberError(const zmq::error_t &, const std::string &) override {}
    };

    class SubscriberCallbackHandler
    {
    private:

        std::promise<bool> promise_;
        std::atomic_int n_messages_;
        const std::string test_string_;
        const std::atomic_int messages_to_receive_;

    public:

        SubscriberCallbackHandler(const std::string &test_string, const int& messages_to_receive) :
            n_messages_(0),
            test_string_(test_string),
            messages_to_receive_(messages_to_receive),
            future_(promise_.get_future())
        {
            this->test_v_.reserve(static_cast<std::size_t>(this->messages_to_receive_));
            this->test_v_.resize(static_cast<std::size_t>(this->messages_to_receive_));
        }

        void handleMsg(const TestData& test_data)
        {
            if (test_data.test_str_ == this->test_string_)
            {
                this->test_v_[test_data.n_msg_] = test_data;
                this->n_messages_++;
                if (this->n_messages_ == this->messages_to_receive_)
                    this->promise_.set_value(true);
            }
            else
            {
                this->promise_.set_value(false);
            }
        }

        std::vector<TestData> test_v_;
        std::future<bool> future_;
    };

    // Publisher configuration variables.
    unsigned publisher_port = 9999;
    std::string publisher_iface = "*";
    std::string publisher_name = "TEST PUBLISHER";
    std::string publisher_version = "1.1.1";
    std::string publisher_info = "This is the TEST publisher";

    // Subscriber configuration variables.
    std::string subscriber_name = "TEST SUBSCRIBER";
    std::string subscriber_version = "1.1.1";
    std::string subscriber_info = "This is the TEST subscriber.";
    std::string publisher_endpoint = "tcp://127.0.0.1:9999";

    // Test data.
    const std::string test_topic = "TEST_TOPIC";
    const std::string test_string = "HOLA MUNDO";
    const int messages_to_receive = 100000;
    std::vector<TestData> test_v;
    std::vector<std::future<void>> publisher_futures;

    // Instanciate the publisher.
    zmqutils::pubsub::PublisherBase publisher(publisher_port, publisher_iface, publisher_name,
                                              publisher_version, publisher_info);

    // Start the publisher.
    bool started = publisher.startPublisher();

    // Check if started.
    if(!started)
    {
        std::cout << "Publisher start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Subscriber and callback handler.
    TestSubscriber subscriber(subscriber_name, subscriber_version, subscriber_info);
    SubscriberCallbackHandler handler(test_string, messages_to_receive);

    // Configure the subscriber.
    subscriber.subscribe(publisher_endpoint);
    subscriber.addTopicFilter(test_topic);

    // Register the callback.
    subscriber.registerCbAndReqProcFunc<std::function<void(const TestData&)>>(
        test_topic, &handler, &SubscriberCallbackHandler::handleMsg);

    // Start the subscriber.
    started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!started)
    {
        std::cout << "Subscriber start failed!!" << std::endl;
        M_FORCE_FAIL()
        return;
    }

    // Prepare the data.
    test_v.reserve(messages_to_receive);
    test_v.resize(messages_to_receive);
    omp_set_num_threads(omp_get_max_threads());
    #pragma omp parallel for
    for (size_t i = 0; i < messages_to_receive; i++)
    {
        test_v[i] = TestData(test_string, static_cast<unsigned>(i));
    }

    // Send the data.
    #pragma omp parallel for
    for (size_t i = 0; i < messages_to_receive; i++)
    {
        publisher.enqueueMsg(test_topic,
                             zmqutils::pubsub::MessagePriority::CriticalPriority,
                             std::move(test_v[i]));
    }

    // Wait all finish.
    handler.future_.wait();
    bool result = handler.future_.get();

    // Stop all.
    publisher.stopPublisher();
    subscriber.stopSubscriber();

    // Check results.
    M_EXPECTED_EQ(result, true)
    M_EXPECTED_EQ(handler.test_v_.size(), static_cast<size_t>(messages_to_receive))
    for (size_t i = 0; i < messages_to_receive; i++)
    {
        M_EXPECTED_EQ(handler.test_v_[i].n_msg_, static_cast<unsigned>(i))
        M_EXPECTED_EQ(handler.test_v_[i].test_str_, test_string)
    }
}

int main()
{
    // Start of the session.
    M_START_UNIT_TEST_SESSION("LibZMQUtils PublisherSubscriber Session")

    // Register the tests.
    M_REGISTER_UNIT_TEST(PublisherSubscriber, BasicPublishSubscribe)
    M_REGISTER_UNIT_TEST(PublisherSubscriber, RegisterCbAndReqProcFunc)
    M_REGISTER_UNIT_TEST(PublisherSubscriber, MultithreadPublishSubscribe)

    // Run the unit tests.
    M_RUN_UNIT_TESTS()
}
