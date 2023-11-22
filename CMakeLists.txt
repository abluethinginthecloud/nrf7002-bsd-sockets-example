#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf_wifi_sta)

target_include_directories(app PUBLIC ${ZEPHYR_BASE}/subsys/net/ip)

target_sources(app PRIVATE
	src/main.c
	src/Task/Led.c
	src/Task/TCP_Client.c
	src/Task/TCP_Server.c
	src/Task/UDP_Client.c
	src/Task/UDP_Server.c
	src/Task/Wifi_Stationing.c

)
