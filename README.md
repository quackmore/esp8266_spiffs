# esp8266_spiffs #

# Summary

Spiffs for ESP8266 NON-OS SDK.
Forked from https://github.com/pellepl/spiffs (thank you so much pellepl) then customized for NON-OS SDK, with no other dependecies.
Including a C++ wrapper to make everything easier.

# Integrating

The repo comes with everything you need for building a full application running tests.

In case you just need SPIFFS use the following files:

        	├── bin
		│   └── upgrade
		│       └── www
		└── src
		    ├── driver
		    ├── espbot
		    │   ├── ... 
		    │   └── spiffs_flash_functions.c
		    ├── include
		    │   ├── ...
		    │   ├── spiffs_config.h
		    │   ├── spiffs_flash_functions.h
		    │   ├── spiffs.h
		    │   ├── spiffs_nucleus.h
		    │   └── ...
		    ├── spiffs
		    │   ├── ...
		    │   ├── spiffs_cache.c
		    │   ├── spiffs_check.c
		    │   ├── spiffs_gc.c
		    │   ├── spiffs_hydrogen.c
		    │   └── spiffs_nucleus.c
		    └── user

While if you want to include the C++ wrapper use the following files:

        	├── bin
		│   └── upgrade
		│       └── www
		└── src
		    ├── driver
		    ├── espbot
		    │   ├── esp8266_spiffs.cpp
		    │   ├── esp8266_spiffs_test.cpp
		    │   ├── ... 
		    │   └── spiffs_flash_functions.c
		    ├── include
		    │   ├── esp8266_spiffs.hpp
		    │   ├── ...
		    │   ├── spiffs_config.h
		    │   ├── spiffs_flash_functions.h
		    │   ├── spiffs.h
		    │   ├── spiffs_nucleus.h
		    │   └── ...
		    ├── spiffs
		    │   ├── ...
		    │   ├── spiffs_cache.c
		    │   ├── spiffs_check.c
		    │   ├── spiffs_gc.c
		    │   ├── spiffs_hydrogen.c
		    │   └── spiffs_nucleus.c
		    └── user


Change the macros in spiffs_flash_functions.h to disable debug serial output
		#define debug (0)

# Using

Checkout SPIFFS documentation.
And C++ header files for the wrapper.

# Building the binaries 
Checkout my other repo https://github.com/quackmore/esp8266_SDK_structure for details.


# License

In addition to SPIFFS licence esp8266_spiffs comes with a [BEER-WARE] license.

Enjoy.
