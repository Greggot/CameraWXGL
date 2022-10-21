set("LED_BLE_CONNECT"  "GPIO_NUM_16")
set("LED_WIFI_CONNECT" "GPIO_NUM_25")

set("SPEED_CAN"    "250")
set("PIN_CAN_TX"   "GPIO_NUM_21")
set("PIN_CAN_RX"   "GPIO_NUM_22") 

set(BLE ../../LIB/ESP-BLE-PlusPlus)
set(NET ../../LIB/Network)

file(GLOB ble "${BLE}/src/Bluetooth/*.cpp")

#DO NOT TOUCH below...until you know what you are doing
idf_component_register(
SRCS
"main.cpp"

${ble}

"${NET}/src/WiFi/STA.cpp"
"${NET}/src/TCP/TCP.client.cpp"

# Application
# "./src/Diagnostic.${Device}.cpp"
# "./src/Diagnostic.Rubber-Sack.cpp"
# common
"./lib/common/debug.cpp"
"./src/Compression.cpp"

INCLUDE_DIRS 
"${BLE}/inc" "${NET}/inc"

"./inc"
"./lib/common/" 
# "./lib/ServiceProtocol/" 
)

configure_file("./inc/build.h.in" ${CMAKE_CURRENT_SOURCE_DIR}/inc/build.h @ONLY)