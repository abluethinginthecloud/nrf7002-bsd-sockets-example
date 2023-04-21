/*! @file TCP_Client.c
 * @brief Implementation of a TCP Client
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
LOG_MODULE_DECLARE( sta, CONFIG_LOG_DEFAULT_LEVEL );

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <zephyr/net/socket.h>
#include <unistd.h> 


#include "TCP_Client.h"
#include "deviceInformation.h"

//! Stack size for the TCP_CLIENT thread
#define TCP_CLIENT_STACK_SIZE 2*1024
//! TCP_CLIENT thread priority level
#define TCP_CLIENT_PRIORITY 7
//! Time in miliseconds to wait to send the TCP message since the board gets a
// stable IP address
#define TCP_CLIENT_WAIT_TO_SEND_MS 6000

//! TCP Client stack definition
K_THREAD_STACK_DEFINE(TCP_CLIENT_STACK, TCP_CLIENT_STACK_SIZE);
//! Variable to identify the TCP Client thread
static struct k_thread tcpClientThread;

//! TCP message sent by the client.
static uint8_t TCPClientMessage[] =
	"=============================TCP MESSAGE:============================="
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque "
	"sodales lorem lorem, sed congue enim vehicula a. Sed finibus diam sed "
	"odio ultrices pharetra. Nullam dictum arcu ultricies turpis congue, "
	"vel venenatis turpis venenatis. Nam tempus arcu eros, ac congue libero "
	"tristique congue. Proin velit lectus, euismod sit amet quam in, "
	"maximus condimentum urna. Cras vel erat luctus, mattis orci ut, varius "
	"urna. Nam eu lobortis velit."
	"\n"
	"Nullam sit amet diam vel odio sodales cursus vehicula eu arcu. Proin "
	"fringilla, enim nec consectetur mollis, lorem orci interdum nisi, "
	"vitae suscipit nisi mauris eu mi. Proin diam enim, mollis ac rhoncus "
	"vitae, placerat et eros. Suspendisse convallis, ipsum nec rhoncus "
	"aliquam, ex augue ultrices nisl, id aliquet mi diam quis ante. "
	"Pellentesque venenatis ornare ultrices. Quisque et porttitor lectus. "
	"Ut venenatis nunc et urna imperdiet porttitor non laoreet massa. Donec "
	"eleifend eros in mi sagittis egestas. Sed et mi nunc. Nunc vulputate, "
	"mauris non ullamcorper viverra, lorem nulla vulputate diam, et congue "
	"dui velit non erat. Duis interdum leo et ipsum tempor consequat. In "
	"faucibus enim quis purus vulputate nullam."
	"\n";

/*! TCP_Client implements the TCP Client task.
* @brief TCP_Client uses a BSD socket estabilish a connection with a defined
* 		TCP server. 
* 		A sample message will be sent to the IP address defined on the 
*		configuration file as Peer address.
* 		This function is used on an independent thread.
*/
void TCP_Client( void )
{
	int tcpClientSocket;
	struct sockaddr_in serverAddress; 
	int connectionResult;
	uint8_t *pTransmitterBuffer; 
	int sentBytes;
	int missingBytesToSend = sizeof( TCPClientMessage );

	// Starve the thread until a DHCP IP is assigned to the board 
	while( !context.connected ){
		k_msleep( TCP_CLIENT_SLEEP_TIME_MS );
	}

	// Server IPV4 address configuration
	serverAddress.sin_family = AF_INET;				   
	serverAddress.sin_port = htons( TCP_CLIENT_PORT );
	inet_pton( 																\
			AF_INET, CONFIG_NET_CONFIG_PEER_IPV4_ADDR,						\
			&serverAddress.sin_addr );

	// Client socket creation 
	tcpClientSocket = socket( 												\
							serverAddress.sin_family, 						\
							SOCK_STREAM, 									\
							IPPROTO_TCP );
	
	if ( tcpClientSocket < 0 )	{
		LOG_ERR( "TCP Client error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Connection between the TCP client and the server 
	k_msleep( TCP_CLIENT_WAIT_TO_SEND_MS );
	connectionResult = connect( 											\
				tcpClientSocket,						    				\
				( struct sockaddr * )&serverAddress,					    \
				sizeof( serverAddress ));

	if ( connectionResult < 0 )	{
		LOG_ERR( "TCP Client error: connect: %d\n", errno );
		k_sleep( K_FOREVER );
	}
	LOG_INF( "TCP Client connected correctly" );
	
	// Send the TCP message by brusts
	pTransmitterBuffer = TCPClientMessage;
	do {
		sentBytes = send( 													\
						tcpClientSocket,									\
						TCPClientMessage,									\
						sizeof( TCPClientMessage ),							\
						0 );
        
		if ( sentBytes < 0 ) {
			LOG_ERR( "TCP Client error: send: %d\n", errno );
			close( tcpClientSocket );
			LOG_ERR( "TCP Client error Connection closed\n" );
		}

		LOG_INF( "TCP Client mode. Sent: %d", sentBytes);
		pTransmitterBuffer += sentBytes;
		missingBytesToSend -= sentBytes;

	} while ( missingBytesToSend );
}

/*! Task_TCP_Client_Init initializes the task TCP Client
*
* @brief TCP Client initialization
*/
void Task_TCP_Client_Init( void ){
	
	k_thread_create	(														\
					&tcpClientThread,										\
					TCP_CLIENT_STACK,										\
					TCP_CLIENT_STACK_SIZE,									\
					(k_thread_entry_t)TCP_Client,							\
					NULL,													\
					NULL,													\
					NULL,													\
					TCP_CLIENT_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&tcpClientThread, "tcpClient");
	 k_thread_start(&tcpClientThread);


};

