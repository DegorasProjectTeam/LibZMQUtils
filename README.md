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
    A C++ libre library with ZMQ related useful utilities. 
    <br />
    <br />
    Developed under the context of Degoras Project for the Spanish Navy Observatory SLR 
    <br />
    (SFEL) station in San Fernando and, of course, any other station that wants to use it!
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

This project is compiled under C++17 with Mingw compilator in Windows OS, using ZMQ library version 4.3.4. It is still pending to test this library with other compilators, OS and ZMQ library versions.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Utilities

This library contains an abstract command server and an abstract command client which can be used to implement your application specific command client and server. The base clasess will deal for you with connection/disconnection, clients present or dead and data sending and receiving, using ZMQ technology. You will only have to deal with the serialization/deserialization of your application's commands data, as well, as with the execution of your custom commands. In this section you can see a brief description of the utilities included in this library. For more specific information, please read the documentation.

### CommandServer / CommandClient

The main classes of this library are CommandServerBase and CommandClientBase. These are abstract classes and must be extended by your specific application server and client class to implement the necessary functionalities. You can see more information about the implementation in the documentation. 

#### CommandClientBase

In the case of the client, you only have to implement the onSendCommand method, that is called after a command is sent. Optionally, you can leave the implementation body blank or use it for log purposes.

You will also have to implement the serialization of the input data before using the client sendCommand method, and the deserialization and management of the data received.

#### CommandServerBase

The CommandServerBase will only deal with the three base commands: connect, disconnect and keepalive. These commands' id are reserved and cannot be used for your application's custom commands. 

You will have to implement the management of your custom commands in the onCustomCommandReceived method. This method will be called whenever a custom command is sent to the server, so you can process the input data and send back the output data.

Aside from the onCustomCommandReceived method, you also have to implement the callbacks onServerStart, onServerStop, onWaitingCommand, onDeadClient, onConnected, onDisconnected, onCommandReceived, onInvalidMsgReceived, onSendingResponse and onServerError. These methods are optional and can be left with an empty implementation.

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
