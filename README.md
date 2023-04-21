# nRF7002dk TCP/UDP Sockets example

The nRF7002dk TCP/UDP Sockets example implements four different kinds of TCP/UDP connections running in parallel, with a previous WiFi Stationing task.


# Overview
This sample implements different tasks. Firstly, it is focused on configuring the board to operate as a WiFi Station, establishing a connection with a router to enable internet connectivity. The status of the connection will be checked continuously, and, once the board is connected to the Wifi network, one of the LEDs of the board will blink. At this moment, the board can create four kinds of connections:

- TCP Client connection. Sends a message to a defined server by TCP.

- TCP Server connection. Receives TCP messages from different clients and echoes them.

- UDP Client communication. Sends a message to a defined server by UDP.

- UDP Server communication. Receives UDP messages from different clients and echoes them.


# Requirments
To use this project it is necessary the Nordic's development kit nRF7002dk, and a WiFi router with stable Internet connection.

The software requirements (i.e. the specific software versions) are detailed on our [“Getting started with nRF7002dk”](https://abluethinginthecloud.com/getting-started-with-nrf7002/) tutorial. It is strongly recommended to read it because it includes instructions to install the IDE and make the necessary configurations to work with the Nordic's development kits.

In addition, to test the example, to test the example, it is recommended to use any software able to create TCP or UDP sessions, for example, Docklight Scripting.

# Testing

To test the examples, the first thing to do is configuring the prj.conf file. This file contains a list of settings the board will implement, and some of them need to be changed depending on the WiFi network configuration, or the desired IPs of the board and the servers.

### WiFi Stationing configurations.
- Security level configurations. Select the security level of the router (on the example above, WPA2) and comment the other options.

- CONFIG_STA_SAMPLE_SSID. Introduce the name of the WiFi network.

- CONFIG_STA_SAMPLE_PASSWORD. Introduces the password of the WiFi network.

### Server’s IPv4 configurations.

- CONFIG_NET_CONFIG_PEER_IPV4_ADDR. IPv4 Address of the servers the clients will connect to.

### Board’s IPv4 configurations.

- CONFIG_NET_DHCPV4. If ‘y’, enables the DHCP assignment. If this option is enabled, the IPv4 address assigned to the board will be shown while running it.

- Static IPv4 configuration. To assign a static IPv4 to the board, disable the CONFIG_NET_DHCPV4 option and uncomment the three following configurations, introducing the desired IP address, the netmask, and the gateway.

Then, build the example with the nrf7002dk configuration, and flash it. To test the working of the TCP/UDP connections, it is proposed to create four sessions with Docklight Scripting, connecting each one with one of the sockets created on the board.



