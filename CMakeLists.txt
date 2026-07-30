cmake_minimum_required(VERSION 3.16)
project(xrealAirDeviceKit C)

set(CMAKE_C_STANDARD 17)

# xrDeviceKit is vendored as a real submodule (modules/xrDeviceKit) so a fresh
# clone of this repo is self-sufficient. nrealAirLinuxDriver instead defaults
# to a sibling directory, since it's also this repo's *own* upstream when used
# standalone; either path is overridable so a parent build (e.g.
# nrealAirLinuxDriver's own examples/, which pulls this package in as a
# submodule) can point it at an already-checked-out copy instead.
if(NOT DEFINED XRDEVICEKIT_ROOT)
	set(XRDEVICEKIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules/xrDeviceKit")
endif()

if(NOT DEFINED NREAL_AIR_DRIVER_ROOT)
	set(NREAL_AIR_DRIVER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../nrealAirLinuxDriver")
endif()

set(NREAL_AIR_HID_DIR "${NREAL_AIR_DRIVER_ROOT}/interface_lib")

if(NOT TARGET xrDeviceKit)
	add_subdirectory("${XRDEVICEKIT_ROOT}" xrDeviceKit)
endif()

if(NOT TARGET xrealAirLibrary)
	add_subdirectory("${NREAL_AIR_HID_DIR}" nrealAirHid)
endif()

add_library(
		xrealAirDeviceKit
		src/imu_protocol_hid.c
		src/mcu_protocol_hid.c
		src/xreal_air_devices.c
)

target_compile_options(xrealAirDeviceKit PRIVATE -fPIC)

target_include_directories(xrealAirDeviceKit
		BEFORE PUBLIC include
)

target_include_directories(xrealAirDeviceKit
		SYSTEM BEFORE PRIVATE
		${NREAL_AIR_HID_DIR}/modules/hidapi
)

target_link_libraries(xrealAirDeviceKit
		PUBLIC xrDeviceKit xrealAirLibrary
		PRIVATE hidapi::hidapi
)

set(XREALAIRDEVICEKIT_INCLUDE_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${XRDEVICEKIT_INCLUDE_DIR}"
		"${NREAL_AIR_HID_DIR}/include"
		PARENT_SCOPE
)
set(XREALAIRDEVICEKIT_LIBRARY xrealAirDeviceKit PARENT_SCOPE)
