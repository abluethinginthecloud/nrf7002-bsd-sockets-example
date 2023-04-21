 #! @file CMakeLists.txt
 #* @brief CMake configuration file
 #
 # @author A BLUE THING IN THE CLOUD S.L.U.
 #    ===  When the technology becomes art ===
 #
 # http://abluethinginthecloud.com
 # j.longares@abluethinginthecloud
 #
 # (c) A BLUE THING IN THE CLOUD S.L.U.
 #
 #
 #
 #       ██████████████    ██    ██    ██  ██████    ██████████████
 #       ██          ██      ████████████████  ██    ██          ██
 #       ██  ██████  ██  ██████  ██    ██        ██  ██  ██████  ██
 #       ██  ██████  ██    ██████    ██      ██      ██  ██████  ██
 #       ██  ██████  ██      ██      ████  ██████    ██  ██████  ██
 #       ██          ██    ██      ██████    ████    ██          ██
 #       ██████████████  ██  ██  ██  ██  ██  ██  ██  ██████████████
 #                       ██████  ████  ██████  ████
 #       ██████  ██████████  ████    ████████      ████      ██
 #       ██  ████  ██    ██  ████        ████    ████████  ██    ██
 #           ██  ██  ████  ██      ██      ██      ██  ████  ██████
 #       ████  ████    ██      ██          ████  ██  ██        ██
 #           ██████████          ██      ██    ██  ████    ██  ████
 #         ██  ████    ██      ██████    ██  ██████████    ██    ██
 #       ██  ████  ████████████████  ██    ██        ████████  ████
 #               ████        ██  ██████  ██████████      ████  ██
 #       ██████  ████████████████    ████  ██    ██████    ██  ████
 #           ████████  ██████  ██    ██████      ██        ████  ██
 #       ██    ██  ████████  ██    ██        ██    ██          ████
 #         ████  ████          ██      ████████████  ██  ████  ██
 #       ██  ██████  ████  ██    ██      ████    ██████████
 #                       ██    ██████    ██      ██      ██  ██████
 #       ██████████████  ██  ██████  ██  ████  ████  ██  ████  ████
 #       ██          ██  ██      ████████  ██    ██      ████  ████
 #       ██  ██████  ██  ████  ██    ██████      ██████████    ████
 #       ██  ██████  ██    ██████    ██  ██  ████      ████  ██████
 #       ██  ██████  ██  ████      ██    ████  ██        ████    ██
 #       ██          ██  ██    ██      ██████████████  ██      ██
 #       ██████████████  ██████  ██        ██  ████    ██████  ████
 # 

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_wifi_sta)

target_include_directories(app PUBLIC ${ZEPHYR_BASE}/subsys/net/ip)

target_sources(app PRIVATE
	src/main.c
)
target_sources_ifdef(CONFIG_TCP_SERVER app PRIVATE src/Task/TCP_Server.c) 
target_sources_ifdef(CONFIG_TCP_CLIENT app PRIVATE src/Task/TCP_Client.c) 
target_sources_ifdef(CONFIG_UDP_SERVER app PRIVATE src/Task/UDP_Server.c) 
target_sources_ifdef(CONFIG_UDP_CLIENT app PRIVATE src/Task/UDP_Client.c) 
target_sources_ifdef(CONFIG_LED app PRIVATE src/Task/Led.c) 
target_sources_ifdef(CONFIG_WIFI app PRIVATE src/Task/Wifi_Stationing.c) 