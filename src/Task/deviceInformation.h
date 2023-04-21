/*! @file deviceInformation.h
 * @brief Relevant information about the device configuration
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

#include <stdio.h>

//! TCP Client port
#define TCP_CLIENT_PORT 504
//! TCP client connection check interval in miliseconds
#define TCP_CLIENT_SLEEP_TIME_MS 100

//! TCP Server port
#define TCP_SERVER_PORT 4242
//! TCP server connection check interval in miliseconds
#define TCP_SERVER_SLEEP_TIME_MS 10

//! UDP Client port
#define UDP_CLIENT_PORT 503
//! UDP client connection check interval in miliseconds
#define UDP_CLIENT_SLEEP_TIME_MS 100

//! UDP Server port
#define UDP_SERVER_PORT 4243
//! UDP server connection check interval in miliseconds
#define UDP_SERVER_SLEEP_TIME_MS 10

/*! @struct sContext
 * @brief Data structure with the Wifi stationing context information
 * @typedef tContext
 * @brief Data type with the Wifi stationing context information
*/
typedef struct sContext{
	//! Pointer to the sell
	const struct shell *pShell;
	union {
		struct {
			//! Connected 
			uint8_t connected					    : 					      1;
			uint8_t connect_result				    : 					      1;
			uint8_t disconnect_requested		    : 					      1;
			uint8_t _unused						    : 					      5;
		};
		uint8_t all;
	};
}tContext;

//! Contains the contex information of the Wifi stationing
extern tContext context;