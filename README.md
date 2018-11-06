# esp8266_SDK_structure #

# Summary

Empty project structure for ESP8266 app based on NON-OS SDK.

# Using

## Building the binaries 
Basic:
	# well this is easy ...
	make

While for building both APP1 and APP2 and make them available for FOTA
	# this is boring ...
	export APP=1
	make clean
	make
	cp bin/upgrade/user1.4096.new.4.bin bin/upgrade/www/user1.bin
	export APP=2
	make clean
	make
	cp bin/upgrade/user2.4096.new.4.bin bin/upgrade/www/user2.bin
	make | grep VERSION: | awk '{print $2}' >bin/upgrade/www/version.txt

## Flashing ESP8266

## Preparing the flash

	# this is an example, customize it to your needs
	# clear the flash
	esptool.py --port /dev/ttyUSB0 erase_flash

	# setup the flash with boot and SDK parameters
	esptool.py --port /dev/ttyUSB0 write_flash -fm dio -fs 32m -ff 40m 0x00000 <your path>/ESP8266_NONOS_SDK/bin/boot_v1.7.bin 0x3FB000 <your path>/ESP8266_NONOS_SDK/bin/blank.bin 0x3FC000 <your path>/ESP8266_NONOS_SDK/bin/esp_init_data_default_v08.bin 0x3FE000 <your path>/ESP8266_NONOS_SDK/bin/blank.bin

## Flashing the app

	# this is an example, customize it to your needs
	esptool.py --port /dev/ttyUSB0 write_flash -fm dio -fs 32m -ff 40m 0x01000 bin/upgrade/user1.4096.new.4.bin

# License

esp8266_SDK_structure comes with a [BEER-WARE] license.

Enjoy.
