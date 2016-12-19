/*    
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2016, Thomas Scheffler
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * \file
 *         Header files for a modified MQTT-Client
 * \author
 *         Thomas Scheffler <scheffler@beuth-hochschule.de>
 */
#define MQTT_CLIENT_H_
/*---------------------------------------------------------------------------*/
#define MQTT_CLIENT_CONFIG_ORG_ID_LEN        32
#define MQTT_CLIENT_CONFIG_TYPE_ID_LEN       32
#define MQTT_CLIENT_CONFIG_AUTH_TOKEN_LEN    32
#define MQTT_CLIENT_CONFIG_EVENT_TYPE_ID_LEN 32
#define MQTT_CLIENT_CONFIG_CMD_TYPE_LEN       8
#define MQTT_CLIENT_CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
#define MQTT_CLIENT_PUBLISH_INTERVAL_MAX      86400 /* secs: 1 day */
#define MQTT_CLIENT_PUBLISH_INTERVAL_MIN          5 /* secs */
/*---------------------------------------------------------------------------*/
/* Default configuration values */
#define CC26XX_WEB_DEMO_DEFAULT_ORG_ID              "quickstart"
#if CPU_FAMILY_CC13XX
#define CC26XX_WEB_DEMO_DEFAULT_TYPE_ID             "cc13xx"
#else
#define CC26XX_WEB_DEMO_DEFAULT_TYPE_ID             "cc26xx"
#endif
#define CC26XX_WEB_DEMO_DEFAULT_EVENT_TYPE_ID       "status"
#define CC26XX_WEB_DEMO_DEFAULT_SUBSCRIBE_CMD_TYPE  "+"
#define CC26XX_WEB_DEMO_DEFAULT_BROKER_PORT         1883
#define CC26XX_WEB_DEMO_DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)
#define CC26XX_WEB_DEMO_DEFAULT_KEEP_ALIVE_TIMER    60
#define CC26XX_WEB_DEMO_DEFAULT_RSSI_MEAS_INTERVAL  (CLOCK_SECOND * 30)


#define CC26XX_WEB_DEMO_NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 3)
#define BOARD_STRING "BeagleBoard"
/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_client_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
typedef struct mqtt_client_config {
  char org_id[MQTT_CLIENT_CONFIG_ORG_ID_LEN];
  char type_id[MQTT_CLIENT_CONFIG_TYPE_ID_LEN];
  char auth_token[MQTT_CLIENT_CONFIG_AUTH_TOKEN_LEN];
  char event_type_id[MQTT_CLIENT_CONFIG_EVENT_TYPE_ID_LEN];
  char broker_ip[MQTT_CLIENT_CONFIG_IP_ADDR_STR_LEN];
  char cmd_type[MQTT_CLIENT_CONFIG_CMD_TYPE_LEN];
  clock_time_t pub_interval;
  int def_rt_ping_interval;
  uint16_t broker_port;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
#endif /* MQTT_CLIENT_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
