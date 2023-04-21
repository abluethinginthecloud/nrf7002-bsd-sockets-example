/*! @file TCP_Server.c
 * @brief Implementation of a TCP Server
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
LOG_MODULE_DECLARE(sta, CONFIG_LOG_DEFAULT_LEVEL);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <zephyr/net/socket.h>
#include <unistd.h> 

#include "TCP_Server.h"
#include "deviceInformation.h"

//! Stack size for the TCP_SERVER thread
#define TCP_SERVER_STACK_SIZE 2*1024
//! TCP_SERVER thread priority level
#define TCP_SERVER_PRIORITY 7

//! TCP Server stack definition
K_THREAD_STACK_DEFINE(TCP_SERVER_STACK, TCP_SERVER_STACK_SIZE);
//! Variable to identify the TCP Server thread
static struct k_thread tcpServerThread;
//! Receiver buffer
static uint8_t receiverBuffer[128];

/*! TCP_Server implements the TCP Server task.
*
* @brief TCP_Server uses a BSD socket to accept TCP connections from 
*		different clients, and echo their messages. 
*  		When the clients send messages to the already known board IP 
*		address, the TCP_Server thread will re-send them their messages.  
* 		This function is used on an independent thread.
*/
void TCP_Server( void ){
	int tcpServerSocket;
	int tcpClientSocket;
	struct sockaddr_in bindAddress;
	int connectionsNumber = 0;
	int bindingResult;
	struct sockaddr_in clientAddress;
	socklen_t clientAddressLength = sizeof( clientAddress );
	uint8_t addressString[32];
	int receivedBytes;
	uint8_t *pTransmitterBuffer;
	int sentBytes; 

	// Starve the thread until a DHCP IP is assigned to the board 
	while( !context.connected ){
		k_msleep( TCP_SERVER_SLEEP_TIME_MS );
	}

	// Server socket creation 
	tcpServerSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( tcpServerSocket < 0 )	{
		LOG_ERR( "TCP Server error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	// Binding 
	bindAddress.sin_family = AF_INET;				   
	bindAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
	bindAddress.sin_port = htons(TCP_SERVER_PORT);	

	bindingResult = bind( 													\
						tcpServerSocket, 									\
						( struct sockaddr * )&bindAddress,					\
						sizeof( bindAddress ));

	if ( bindingResult < 0 )	{
		LOG_ERR( "TCP Server error: bind: %d\n", errno );
		k_sleep( K_FOREVER );
	}
    
	/* Listen */
	if ( listen( tcpServerSocket, 5 ) < 0 )	{
		LOG_ERR( "TCP Server error: listen: %d\n", errno );
		k_sleep( K_FOREVER );
	}

	LOG_INF( "TCP server waits for a connection on port %d...",				\
		   TCP_SERVER_PORT );

	while ( 1 ) {
		// Accept connection 
		tcpClientSocket = accept( 											\
								tcpServerSocket, 							\
								( struct sockaddr * )&clientAddress,		\
								&clientAddressLength );

		if ( tcpClientSocket < 0 ) {
			LOG_ERR( "TCP Server error: accept: %d\n", errno );
			continue;
		}

		// Convert network address from internal to numeric ASCII form
		inet_ntop( 															\
				clientAddress.sin_family,									\
				&clientAddress.sin_addr,									\
				addressString, 												\
				sizeof( addressString ));
		LOG_INF( "TCP Server: Connection #%d from %s",						\
				connectionsNumber++, addressString );

		// Receiving loop
		while ( 1 ) {
			
			// Receive 
			receivedBytes = recv( 											\
								tcpClientSocket, 							\
								( char *) receiverBuffer, 					\
								sizeof( receiverBuffer ),					\
								0 );			

			if ( receivedBytes <= 0 ) {
				if ( receivedBytes < 0 ) { 
						LOG_ERR( "TCP Server error: recv: %d\n", errno );
				}
				break;
			}

			// Send 
			pTransmitterBuffer = receiverBuffer;
			do {
				// Sending the received message by brusts
				sentBytes = send( 											\
								tcpClientSocket, 							\
								pTransmitterBuffer, 						\
								receivedBytes,								\
								0 );

				if ( sentBytes < 0 ) {
					LOG_ERR( "TCP Server error: send: %d\n", errno );
					close( tcpClientSocket );
					LOG_ERR( "TCP server: Connection from %s closed\n", 
							addressString );
				}
				pTransmitterBuffer += sentBytes;
				receivedBytes -= sentBytes;
				
            	LOG_INF( "TCP Server mode. Received and replied with %d "	\
						"bytes", sentBytes );
			} while ( receivedBytes );
		}
	}
}

/*! Task_TCP_Server_Init initializes the task TCP Server
*
* @brief TCP Server initialization
*/
void Task_TCP_Server_Init( void ){
	k_thread_create	(														\
					&tcpServerThread,										\
					TCP_SERVER_STACK,										\
					TCP_SERVER_STACK_SIZE,									\
					(k_thread_entry_t)TCP_Server,							\
					NULL,													\
					NULL,													\
					NULL,													\
					TCP_SERVER_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&tcpServerThread, "tcpServer");
	 k_thread_start(&tcpServerThread);
}
