<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Cplusplus][cplusplus-shield]][cplusplus-url]
[![EUPL License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<!-- PROJECT LOGO -->
  <h1 align="center">LibZMQUtils - ZMQ Utilitites Library</h1>

  <p align="center">
    <br />
A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure. The library is suited for the quick and easy integration of new and old systems and can be used in different sectors and disciplines seeking robust messaging and serialization solutions.
    <br />
    <br />
Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!    
    <br />
    <br />
    <br />
    <a href="https://github.com/DegorasProjectTeam/LibZMQUtils/tree/master/LibZMQUtils/docs"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/DegorasProjectTeam/LibZMQUtils/tree/master/LibZMQUtils/examples">View Examples</a>
    ·
    <a href="https://github.com/DegorasProjectTeam/LibZMQUtils/issues">Report Bug</a>
    ·
    <a href="https://github.com/othneildrew/Best-README-Template/issues">Request Feature</a>
  </p>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project

This project was born with the aim to provide a generic command client/server infrastructure, so your application can send commands to other applications (or even to itself) and receive the result of the command. 

### Built With

This project is compiled under C++17 with Mingw compilator in Windows OS, using ZMQ library version 4.3.4. Also works under Ubuntu 22 using gcc13. It is still pending to test this library with other compilators, OS and ZMQ library versions.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Utilities

This library contains the implementation of Client/Server and Publisher/Subscriber patterns using the ZMQ framework.

The Client/Server module contains an abstract command server and an abstract command client which can be used to implement your application specific command client and server. The base clasess will deal for you with connection/disconnection, clients present or dead and data sending and receiving. You will only have to deal with the serialization/deserialization of your application's commands data, as well, as with the execution of your custom commands.

The Publisher/Subscriber module contains a base publisher, that can be used directly or subclassed, to send messages to subscribers. The subscriber class is abstract and must be subclassed. The subscriber can subscribe to multiple publishers and filter the received messages by topics.

In this section you can see a brief description of both modules included in this library. For more specific information, please read the documentation.

### CommandServer / CommandClient

The main classes of this library are CommandServerBase and CommandClientBase. These are abstract classes and must be extended by your specific application server and client class to implement the necessary functionalities. You can see more information about the implementation in the documentation. 

#### CommandClientBase

In the case of the client, you have to implement the onClientStart, onClientStop, onWaitingReply, onDeadServer, onConnected, onDisconnected, onInvalidMsgReceived, onReplyReceived, onSendingCommand and onClientError methods. You can add the logic for your application when those events occur, leave the implementation body blank or use it for log purposes.

You will also have to implement the serialization of the input data before using the client sendCommand method, and the deserialization and management of the data received.

#### CommandServerBase

The CommandServerBase will only deal with the three base commands: connect, disconnect and keepalive. These commands' IDs are reserved and cannot be used for your application's custom commands. 

You will have to implement the management of your custom commands in the onCustomCommandReceived method. This method will be called whenever a custom command is sent to the server, so you can process the input data and send back the output data.

Aside from the onCustomCommandReceived method, you also have to implement the callbacks onServerStart, onServerStop, onWaitingCommand, onDeadClient, onConnected, onDisconnected, onCommandReceived, onInvalidMsgReceived, onSendingResponse and onServerError. These methods are optional and can be left with an empty implementation.

### Publisher / Subscriber

The main classes of this library are PublisherBase and SubscriberBase. SubscriberBase is an abstract class and must be extended by your specific application subscriber class to implement the necessary functionalities. PublisherBase can be used directly or subclassed. You can see more information about the implementation in the documentation. 

#### PublisherBase

In the case of the client, you can implement the onPublisherStart, onPublisherStop, onSendingMsg and onPublisherError methods. The base class' implementation does nothing.

You will also have to implement the serialization of the input data before using the publisher sendMsg method.

#### SubscriberBase

The SubscriberBase can filter any number of topics, so that it will only receive those topics that are allowed in by using the addTopicFilter method. There is one reserved topic, denoted by constant kReservedExitTopic. This topic cannot be set or unset.

You will have to implement the callbacks onSubscriberStart, onSubscriberStop, onMsgReceived, onInvalidMsgReceived, and onSubscriberError. These methods can be left with an empty implementation.

### Generic and helper utilities

The utilities are included in namespace zmqutils::utils. These utilities include functions to serialize/deserialize data, manage date and time, get information from the OS, etc.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Usage and examples

As previously stated, the usage of the utilities in this library basically consists in extending the CommandServerBase and CommandClientBase with the necessary functionalities to deal with your custom commands.

You can see an example implementation for the Amelas Project, developed to command the new SLR station's mount system.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- LICENSE -->
## License

Distributed under the European Union Public License. See `LICENSE` file for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [ZeroMQ](https://zeromq.org/)
* [ZMQ C++ Binging - cppzmq](https://github.com/zeromq/cppzmq)
* [European Union Public License](https://choosealicense.com)
* [Shields](https://shields.io)
* [Best-README-Template](https://github.com/othneildrew/Best-README-Template)

<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[cplusplus-shield]: https://img.shields.io/badge/-C++17-black?style=for-the-badge&logo=cplusplus&colorB=555
[cplusplus-url]: https://en.cppreference.com/w/cpp/17
[license-shield]: https://img.shields.io/badge/EUPL%201.2-green.svg?style=for-the-badge
[license-url]: https://eupl.eu/
[linkedin-shield]: https://img.shields.io/badge/LinkedIn-blue?style=for-the-badge&logo=linkedin
[linkedin-url]: https://www.linkedin.com/in/angelveraherrera/
