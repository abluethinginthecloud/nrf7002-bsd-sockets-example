/*! @file UDP_Client.c
 * @brief Implementation of a UDP Client
 *
 * @author A BLUE THING IN THE CLOUD S.L.U.
 *    ===  When the technology becomes art ===
 *
 * http://abluethinginthecloud.com
 * j.longares@abluethinginthecloud
 *
 * (c) A BLUE THING IN THE CLOUD S.L.U.
 *
 *
 *
        ██████████████    ██    ██    ██  ██████    ██████████████
        ██          ██      ████████████████  ██    ██          ██
        ██  ██████  ██  ██████  ██    ██        ██  ██  ██████  ██
        ██  ██████  ██    ██████    ██      ██      ██  ██████  ██
        ██  ██████  ██      ██      ████  ██████    ██  ██████  ██
        ██          ██    ██      ██████    ████    ██          ██
        ██████████████  ██  ██  ██  ██  ██  ██  ██  ██████████████
                        ██████  ████  ██████  ████
        ██████  ██████████  ████    ████████      ████      ██
        ██  ████  ██    ██  ████        ████    ████████  ██    ██
            ██  ██  ████  ██      ██      ██      ██  ████  ██████
        ████  ████    ██      ██          ████  ██  ██        ██
            ██████████          ██      ██    ██  ████    ██  ████
          ██  ████    ██      ██████    ██  ██████████    ██    ██
        ██  ████  ████████████████  ██    ██        ████████  ████
                ████        ██  ██████  ██████████      ████  ██
        ██████  ████████████████    ████  ██    ██████    ██  ████
            ████████  ██████  ██    ██████      ██        ████  ██
        ██    ██  ████████  ██    ██        ██    ██          ████
          ████  ████          ██      ████████████  ██  ████  ██
        ██  ██████  ████  ██    ██      ████    ██████████
                        ██    ██████    ██      ██      ██  ██████
        ██████████████  ██  ██████  ██  ████  ████  ██  ████  ████
        ██          ██  ██      ████████  ██    ██      ████  ████
        ██  ██████  ██  ████  ██    ██████      ██████████    ████
        ██  ██████  ██    ██████    ██  ██  ████      ████  ██████
        ██  ██████  ██  ████      ██    ████  ██        ████    ██
        ██          ██  ██    ██      ██████████████  ██      ██
        ██████████████  ██████  ██        ██  ████    ██████  ████



*/
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, LOG_LEVEL_DBG);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>

//#include <arpa/inet.h>
#include <zephyr/net/socket.h>
#include <zephyr/sys/fdtable.h>
#include <unistd.h> 

#include "UDP_Client.h"
#include "deviceInformation.h"

//! Stack size for the UDP_SERVER thread
#define UDP_CLIENT_STACK_SIZE 2*1024
//! UDP_SERVER thread priority level
#define UDP_SERVER_PRIORITY 7
//! Time in miliseconds to wait to send the UDP message since the board 
// gets a stable IP address
#define UDP_CLIENT_WAIT_TO_SEND_MS 5000

//! UDP Client stack definition
K_THREAD_STACK_DEFINE(UDP_CLIENT_STACK, UDP_CLIENT_STACK_SIZE);
//! Variable to identify the UDP Client thread
static struct k_thread udpClientThread;
static int udpClientSocket;

//! UDP message sent by the client.
static const uint8_t udpClientMessage[] =
    "UDP Message\n";


/*! UDP_Client implements the UDP Client task.
* @brief UDP_Client uses a BSD socket to send messages to a defined UDP
*		server. 
*		A sample message will be sent to the IP address defined on the 
*		configuration file as Peer address.
* 		This function is used on an independent thread.
*/
void UDP_Client() {
	struct sockaddr_in serverAddress;
	int connectionResult;
	int sentBytes = 0;
	int errorCode = 0;
	
	// Starve the thread until a DHCP IP is assigned to the board 
    while( !context.connected ){
		k_msleep( UDP_CLIENT_SLEEP_TIME_MS );
	}

	// Server IPV4 address configuration 
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons( UDP_CLIENT_PORT );
    errorCode = inet_pton( 																\
			AF_INET, 														\
			CONFIG_NET_CONFIG_PEER_IPV4_ADDR, 								\
			&serverAddress.sin_addr );

	// Client socket creation 
	//z_free_fd(1);
	udpClientSocket = socket( 												\
							serverAddress.sin_family, 						\
							SOCK_DGRAM, 									\
							IPPROTO_UDP );									\

	if ( udpClientSocket < 0 ) {
		LOG_ERR( "UDP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Connection to the server. 
	connectionResult = connect( 											\
							udpClientSocket, 								\
							( struct sockaddr * )&serverAddress, 			\
							sizeof( serverAddress ));

	if ( connectionResult < 0 ) {
		LOG_ERR( "UDP Client error: connect: %d\n", errno );
		k_sleep( K_FOREVER );
	}
	LOG_INF( "UDP Client connected correctly" );

	// Send the udp message 
	k_msleep( UDP_CLIENT_WAIT_TO_SEND_MS );
	sentBytes = send( 														\
					udpClientSocket, 										\
					udpClientMessage, 										\
					sizeof( udpClientMessage ), 							\
					0 );

	LOG_INF( "UDP Client mode. Sent: %d", sentBytes );
	if ( sentBytes < 0 ) {
		LOG_ERR( "UDP Client error: send: %d\n", errno );
		close( udpClientSocket );
		LOG_ERR( "UDP Client error Connection from closed\n" );
	}
	
}

/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief UDP Client initialization
*/
void Task_UDP_Client_Init( void ){
	k_thread_create	(														\
					&udpClientThread,										\
					UDP_CLIENT_STACK,										\
					UDP_CLIENT_STACK_SIZE,									\
					(k_thread_entry_t)UDP_Client,							\
					NULL,													\
					NULL,													\
					NULL,													\
					UDP_SERVER_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&udpClientThread, "udpClient");
	 k_thread_start(&udpClientThread);
}



