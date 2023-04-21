/*! @file Led.c
 * @brief Toggles LED0 when WIFI is connected
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
#include <zephyr/drivers/gpio.h>

#include "Led.h"
#include "deviceInformation.h"

//! Led0 blink period in miliseconds
#define LED_SLEEP_TIME_MS   100
//! Devicetree identifier for the "led0" alias
#define LED0_NODE DT_ALIAS(led0)
//! Stack size for the LED thread
#define LED_STACK_SIZE 512
//! LED thread priority level
#define LED_PRIORITY 7

//! LED stack definition
K_THREAD_STACK_DEFINE(LED_STACK, LED_STACK_SIZE);
//! Variable to identify the LED thread
static struct k_thread ledThread;
//! Specifies where the LED0 is on the devicetree files
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);


/*! Toggle_Led implements the Toggle Led task.
* @brief Toggle_Led turns on and off the LED0 on the board depending on the
* 		status of the Wifi connection. When the board is ready to receive 
*		TCP or UDP connections, the LED will blink. 
* 		This function is used on an independent thread.
*/
void Toggle_Led( void ) {
	int returnCode;

	if( !device_is_ready ( led.port ) ) {
		LOG_ERR( "LED device is not ready" );
		return;
	}

	returnCode = gpio_pin_configure_dt( &led, GPIO_OUTPUT_ACTIVE );
	if( returnCode < 0 ) {
		LOG_ERR( "Error %d: failed to configure LED pin", returnCode );
		return;
	}

	while( 1 ) {
		if( context.connected ) {
			gpio_pin_toggle_dt( &led );
			k_msleep ( LED_SLEEP_TIME_MS );
		} else {
			gpio_pin_set_dt( &led, 0 );
			k_msleep( LED_SLEEP_TIME_MS );
		}
	}
}

/*! Task_Toggle_Led_Init initializes the task Toggle Led
*
* @brief Toggle Led initialization
*/
void Task_Toggle_Led_Init( void ){
    
	k_thread_create	(														\
					&ledThread,										        \
					LED_STACK,										        \
					LED_STACK_SIZE,									        \
					(k_thread_entry_t)Toggle_Led,							\
					NULL,													\
					NULL,													\
					NULL,													\
					LED_PRIORITY,									        \
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&ledThread, "toggleLed");
	 k_thread_start(&ledThread);
}