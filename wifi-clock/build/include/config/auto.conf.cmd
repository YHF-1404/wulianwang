deps_config := \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/console/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/esp_http_client/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/esp_http_server/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/log/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/mqtt/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/spiffs/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/util/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/vfs/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/wifi_provisioning/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/home/yhf/my_esp8266_project/wulianwang/wifi-clock/main/Kconfig.projbuild \
	/home/yhf/esp/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/home/yhf/esp/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
