MQTT Client
============================



This is an adaptation of the MQTT Client from Texas Instruments for use on `minimal-net` and `avr-zigbit` plattforms.

It is currently used for experiments with MQTT at the  [IPv6-Lab at the Beuth Hochschule für Technik Berlin, Germany](https://wiki.ipv6lab.beuth-hochschule.de).

#### Publish

The application posts a regular status report in the Whatson IoT format at: 

    iot-2/evt/status/fmt/json

It currently contains a Name, SequenceNr. and Uptime information.


#### Subscribe
The application subscribes to the following MQTT topic:

    iot-2/cmd/+/fmt/json
    
An LED can be controlled by publishing `0` or `1` to the following topic:
    
    iot-2/cmd/leds/fmt/json 

### Compiling for platform 'minimal-net'
The code is compiled with the following settings in `platform/minimal-net/contiki-conf.h`

    #undef UIP_CONF_IPV6_RPL 
    #define RPL_BORDER_ROUTER 0
    #define UIP_CONF_IPV6_RPL 0

It publishes to a local MQTT broker, such as `mosquitto` on `aaaa::1`


