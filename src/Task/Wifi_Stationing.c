/*! @file Wifi_Stationing.c
 * @brief Implements the Wifi stationing to the board
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

#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/socket.h>
#include "net_private.h"


#include "Wifi_Stationing.h"
#include "deviceInformation.h"

//! Flag related to the shell events on the TCP Server
#define SHELL_EVENTS_FLAG (NET_EVENT_WIFI_CONNECT_RESULT |		\
				NET_EVENT_WIFI_DISCONNECT_RESULT)
//! Connection timeout in miliseconds
#define TIMEOUT_MS  100
//! Status check interval in miliseconds
#define STATUS_POLLING_MS   300

//! Wifi thread priority level
#define WIFI_STACK_SIZE 4096
//! Wifi thread priority level
#define WIFI_PRIORITY 4

//! WiFi stack definition
K_THREAD_STACK_DEFINE(WIFI_STACK, WIFI_STACK_SIZE);
//! Variable to identify the Wifi thread
static struct k_thread wifiThread;
//! Struct variable with the WiFi callback and flags
static struct net_mgmt_event_callback wifiEventsCallback;
//! Struct variable with the Net event callback and flags
static struct net_mgmt_event_callback netEventsCallback;
//! Variable for the TCP Context
tContext context;

/*! Handle_Wifi_Connect_Result checks the connection status after 
* the connection callback is activated, updating it if it is 
* necessary. 
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
*                   events callback
* @return void
*/
static void Handle_Wifi_Connect_Result(                                     \
	struct net_mgmt_event_callback *callback);

/*! Handle_Wifi_Disconnect_Result checks the connection status after 
* the disconnection callback is activated, updating it if it is 
* necessary. 
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
*                   events callback
* @return void
*/
static void Handle_Wifi_Disconnect_Result(                                  \
	struct net_mgmt_event_callback *callback);

/*! Print_DHCP_IP gets the DHCP IP address assigned to the board, and prints
* it on its serial port.
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
					events callback
* @return void
*/
static void print_dhcp_ip(struct net_mgmt_event_callback *callback);

/*! Wifi_Configuration_Reader reads the configuration arguments recorded on
* the configuration file, and stores them on a struct.
* @param[in] struct wifi_connect_req_params *parameters struct containing 
*                   the	WiFi configuration info.
* @return int, 0 when the function is executed successfully.
*/
static int Wifi_Configuration_Reader( 
							struct wifi_connect_req_params *parameters );

/*! Wifi_Connect runs the connection with the WiFi provider. 
* @param[in] void
* @return int, 0 when the function is executed successfully.
*/
static int Wifi_Connect( void );

/*! Handle_Wifi_Events records when is there a wifi event, such a connection
* or a disconnection, and calls to the corresponding handler function.
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
					events callback
* @param[in] uint32_t managementEvent identifies the kind of network event 
*					(connection or disconnection)
*@param[in] struct net_if *interface information about the net interface
* @return void
*/
static void Handle_Wifi_Events(                                             \
                            struct net_mgmt_event_callback *callback,       \
				     		uint32_t managementEvent,                       \
					 		struct net_if *interface) {
	switch ( managementEvent ) {
	case NET_EVENT_WIFI_CONNECT_RESULT:
		Handle_Wifi_Connect_Result( callback );
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		Handle_Wifi_Disconnect_Result( callback );
		break;
	default:
		break;
	}
}


/*! Handle_Net_Events records when is there a net event, like the assignment
* of the DHCP IP address, and calls to the corresponding handler function.
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
					events callback
* @param[in] uint32_t managementEvent identifies the kind of network event 
*					(connection or disconnection)
* @param[in] struct net_if *interface information about the net interface
* @return void
*/
static void Handle_Net_Events(                                              \
                            struct net_mgmt_event_callback *callback,       \
				    		uint32_t managementEvent,                       \
							struct net_if *interface) {

	switch ( managementEvent ) {
	case NET_EVENT_IPV4_DHCP_BOUND:
		print_dhcp_ip( callback );
		break;
	default:
		break;
	}
}


/*! Cmd_Wifi_Status writes on the NRF serial port information about the 
* connection status when it is working as a WiFi station. For example, 
* it writes down if the board is Scanning, Connected, or Disconnected.
* @param[in] void
* @return int, 0 when the function has ran succesfully
*/
static int Cmd_Wifi_Status( void ) {
	struct net_if *interface = net_if_get_default();
	struct wifi_iface_status status = { 0 };

	if ( net_mgmt(                                                          \
                NET_REQUEST_WIFI_IFACE_STATUS,                              \
                interface,                                                  \
                &status,                                                    \
				sizeof( struct wifi_iface_status ))) {
		LOG_INF("Status request failed");
		return -ENOEXEC;
	}

	LOG_INF( "==================" );
	LOG_INF( "State: %s", wifi_state_txt( status.state ) );

	if ( status.state >= WIFI_STATE_ASSOCIATED ) {
		uint8_t MACStringBuffer[ sizeof( "xx:xx:xx:xx:xx:xx" )];

		LOG_INF( "Interface Mode: %s",                                      \
		       wifi_mode_txt( status.iface_mode ));
		LOG_INF( "Link Mode: %s",                                           \
		       wifi_link_mode_txt( status.link_mode ));
		LOG_INF( "SSID: %-32s", status.ssid );
		LOG_INF( "BSSID: %-32s",                                            \
		       	(char *) net_sprint_ll_addr_buf(                            \
                                                status.bssid,               \
									            WIFI_MAC_ADDR_LEN,          \
									            MACStringBuffer,            \
									            sizeof( MACStringBuffer )));
		LOG_INF( "Band: %s", wifi_band_txt( status.band ));
		LOG_INF( "Channel: %d", status.channel );
		LOG_INF( "Security: %s", wifi_security_txt( status.security ));
		LOG_INF( "MFP: %s", wifi_mfp_txt( status.mfp ));
		LOG_INF( "RSSI: %d", status.rssi );
	}
	return 0;
}

/*! Handle_Wifi_Connect_Result checks the connection status after 
* the connection callback is activated, updating it if it is 
* necessary. 
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
*                    events callback
* @return void
*/
static void Handle_Wifi_Connect_Result(                                     \
	struct net_mgmt_event_callback *callback) {
	const struct wifi_status *status =
							(const struct wifi_status *) callback -> info;

	if ( context.connected ) {
		return;
	}

	if ( status -> status ) {
		LOG_ERR( "Connection failed (%d)", status -> status );
	} else {
		LOG_INF( "Connected" );
		context.connected = true;
	}

	context.connect_result = true;
}

/*! Handle_Wifi_Disconnect_Result checks the connection status after 
* the disconnection callback is activated, updating it if it is 
* necessary. 
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
*                    events callback
* @return void
*/
static void Handle_Wifi_Disconnect_Result(                                  \
	struct net_mgmt_event_callback *callback) {
	const struct wifi_status *status =
							( const struct wifi_status * ) callback -> info;

	if ( !context.connected ) {
		return;
	}

	if ( context.disconnect_requested ) {
		LOG_INF( "Disconnection request %s (%d)",                           \
			 status->status ? "failed" : "done",                            \
					status -> status);
		context.disconnect_requested = false;
	} else {
		LOG_INF( "Received Disconnected" );
		context.connected = false;
	}

	Cmd_Wifi_Status();
}


/*! Print_DHCP_IP gets the DHCP IP address assigned to the board, and prints
* it on its serial port.
* @param[in] struct net_mgmt_event_callback *callback pointer to the net 
					events callback
* @return void
*/
static void print_dhcp_ip(struct net_mgmt_event_callback *callback) {
	const struct net_if_dhcpv4 *dhcpv4 = callback -> info;
	const struct in_addr *address = &dhcpv4 -> requested_ip;
	char dhcpInfo[128];

	net_addr_ntop( AF_INET, address, dhcpInfo, sizeof( dhcpInfo ));

	LOG_INF( "DHCP IP address: %s", dhcpInfo );
}


/*! Wifi_Configuration_Reader reads the configuration arguments recorded on
* the configuration file, and stores them on a struct.
* @param[in] struct wifi_connect_req_params *parameters struct containing 
*                   the WiFi configuration info.
* @return int, 0 when the function is executed successfully.
*/
static int Wifi_Configuration_Reader(                                       \
							struct wifi_connect_req_params *parameters ){
	parameters -> timeout = SYS_FOREVER_MS;

	/* SSID */
	parameters -> ssid = CONFIG_STA_SAMPLE_SSID;
	parameters -> ssid_length = strlen( parameters -> ssid );

#if defined( CONFIG_STA_KEY_MGMT_WPA2 )
	parameters -> security = 1;
#elif defined( CONFIG_STA_KEY_MGMT_WPA2_256 )
	parameters -> security = 2;
#elif defined( CONFIG_STA_KEY_MGMT_WPA3 )
	parameters -> security = 3;
#else
	parameters -> security = 0;
#endif

#if !defined( CONFIG_STA_KEY_MGMT_NONE )
	parameters -> psk = CONFIG_STA_SAMPLE_PASSWORD;
	parameters -> psk_length = strlen( parameters -> psk );
#endif
	parameters -> channel = WIFI_CHANNEL_ANY;

	/* MFP (optional) */
	parameters -> mfp = WIFI_MFP_OPTIONAL;

	return 0;
}

/*! Wifi_Connect runs the connection with the WiFi provider. 
* @param[in] void
* @return int, 0 when the function is executed successfully.
*/
static int Wifi_Connect( void ) {
	struct net_if *interface = net_if_get_default();
	static struct wifi_connect_req_params connectionParameters;

	context.connected = false;
	context.connect_result = false;
	Wifi_Configuration_Reader ( &connectionParameters );

	if (net_mgmt(                                                           \
                NET_REQUEST_WIFI_CONNECT,                                   \
				interface,                                                  \
		     	&connectionParameters,                                      \
				sizeof( struct wifi_connect_req_params ))) {
		LOG_ERR( "Connection request failed" );
		return -ENOEXEC;
	}

	LOG_INF( "Connection requested" );

	return 0;
}

/*! Wifi_Stationing implements the task WiFi Stationing.
* 
* @brief Wifi_Stationing makes the complete connection process, while 
*       printing by LOG commands the connection status.
*       This function is used on an independent thread.
*/
void Wifi_Stationing( void ){
    int i;
	memset( &context, 0, sizeof( context ));

	net_mgmt_init_event_callback(                                           \
                                &wifiEventsCallback,                        \
                                Handle_Wifi_Events,                         \
                                SHELL_EVENTS_FLAG );

	net_mgmt_add_event_callback( &wifiEventsCallback );


	net_mgmt_init_event_callback(                                           \
                                &netEventsCallback,                         \
				                Handle_Net_Events,                          \
				                NET_EVENT_IPV4_DHCP_BOUND );

	net_mgmt_add_event_callback( &netEventsCallback );

	LOG_INF( "Starting %s with CPU frequency: %d MHz",                      \
			CONFIG_BOARD, SystemCoreClock/MHZ( 1 ));
	k_sleep( K_SECONDS( 1 ));

	LOG_INF( "Static IP address (overridable): %s/%s -> %s",                \
		CONFIG_NET_CONFIG_MY_IPV4_ADDR,                                     \
		CONFIG_NET_CONFIG_MY_IPV4_NETMASK,                                  \
		CONFIG_NET_CONFIG_MY_IPV4_GW );     

	while ( 1 ) {
		Wifi_Connect();

		for ( i = 0; i < TIMEOUT_MS; i++ ) {
			k_sleep( K_MSEC( STATUS_POLLING_MS ));
			Cmd_Wifi_Status();
			if ( context.connect_result ) {
				break;
			}
		}
		if ( context.connected ) {
			LOG_INF( "============" );
			k_sleep( K_FOREVER );
		}
		else if ( !context.connect_result ) {
			LOG_ERR( "Connection Timed Out" );
		}
	}
}

/*! Task_Wifi_Stationing_Init initializes the task Wifi Stationing.
*
* @brief Wifi Stationing initialization
*/
void Task_Wifi_Stationing_Init( void ){
    
	k_thread_create	(														\
					&wifiThread,										    \
					WIFI_STACK,										        \
					WIFI_STACK_SIZE,									    \
					(k_thread_entry_t)Wifi_Stationing,						\
					NULL,													\
					NULL,													\
					NULL,													\
					WIFI_PRIORITY,									        \
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&wifiThread, "wifiStationing");
	 k_thread_start(&wifiThread);
}
