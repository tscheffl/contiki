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
 *         A modified MQTT-Client
 * \author
 *         Thomas Scheffler <scheffler@beuth-hochschule.de>
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
//#include "net/ipv6/sicslowpan.h"

#include "mqtt.h"
#include "mqtt-client.h"
#include "sys/etimer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
# ifdef PROJECT_TARGET_AVR_ZIGBIT
#include <avr/io.h>
#endif 

#undef DBG
#define DEBUG_MQTT 0


#if DEBUG_MQTT == 1
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
//#define DBG(...) printf(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
/*
 * IBM server: messaging.quickstart.internetofthings.ibmcloud.com
 * (184.172.124.189) mapped in an NAT64 (prefix 64:ff9b::/96) IPv6 address
 * Note: If not able to connect; lookup the IP address again as it may change.
 *
 * Alternatively, publish to a local MQTT broker (e.g. mosquitto) running on
 * the node that hosts your border router
 */
//#ifdef MQTT_DEMO_BROKER_IP_ADDR
//static const char *broker_ip = MQTT_DEMO_BROKER_IP_ADDR;
//static const char *broker_ip = "aaaa:0000:0000:0000:0a00:27ff:fe1b:3186";
# if PROJECT_TARGET_MINIMAL_NET
static const char *broker_ip = "aaaa:0000:0000:0000:0000:0000:0000:0001";
# else
static const char *broker_ip = "2001:0638:0812:b88a:0000:0000:0000:0002";
# endif /*MINIMAL_NET*/

#define DEFAULT_ORG_ID              "mqtt-demo"
//#else
//static const char *broker_ip = "0064:ff9b:0000:0000:0000:0000:b8ac:7cbd";
//#define DEFAULT_ORG_ID              "quickstart"
//#endif
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topic.
 * Make sure they are large enough to hold the entire respective string
 *
 * d:quickstart:status:EUI64 is 32 bytes long
 * iot-2/evt/status/fmt/json is 25 bytes
 * We also need space for the null termination
 */
#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 512
static struct mqtt_connection conn;
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
#define QUICKSTART "quickstart"
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;
static struct etimer publish_periodic_timer;
static char *buf_ptr;
static uint16_t seq_nr_value = 0;
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
//static struct uip_icmp6_echo_reply_notification echo_reply_notification;
//static struct etimer echo_request_timer;
int def_rt_rssi = 0;
/*---------------------------------------------------------------------------*/
static uip_ip6addr_t def_route;
/*---------------------------------------------------------------------------*/
static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
/*Status of button (ANSolutions Board)*/
static volatile short button_flag = 0; //initial state for debounce-code


/*---------------------------------------------------------------------------*/
PROCESS(mqtt_client_process, "MQTT client");
/*---------------------------------------------------------------------------*/
int
ipaddr_sprintf(char *buf, uint8_t buf_len,
                               const uip_ipaddr_t *addr)
{
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}

/* battery power level*/
/************************/
 
/** \brief function for initialization of the adc*/
void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0)|(1<<REFS1); //intern 2.54V voltage reference
 
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
 
/** \brief adc read function */
uint16_t adc_read()
{
    ADMUX = (ADMUX & 0xF8)|0;     // clears the bottom 3 bits before ORing
    // start single convertion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}
/*Battery Level end*/

/*---------------------------------------------------------------------------*/
//static void
//echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data,
//                   uint16_t datalen)
//{
//  if(uip_ip6addr_cmp(source, uip_ds6_defrt_choose())) {
// thomas:
//    def_rt_rssi = sicslowpan_get_last_rssi();
//  }
//}
/*---------------------------------------------------------------------------*/
static int
construct_pub_topic(void)
{
  int len = snprintf(pub_topic, BUFFER_SIZE, "iot-2/evt/%s/fmt/json",
                     conf.event_type_id);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Pub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_sub_topic(void)
{
  int len = snprintf(sub_topic, BUFFER_SIZE, "iot-2/cmd/%s/fmt/json",
                     conf.cmd_type);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Sub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "d:%s:%s:%02x%02x%02x%02x%02x%02x",
                     conf.org_id, conf.type_id,
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config(void)
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = MQTT_CLIENT_STATE_CONFIG_ERROR;
    return;
  }

  if(construct_sub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = MQTT_CLIENT_STATE_CONFIG_ERROR;
    return;
  }

  if(construct_pub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = MQTT_CLIENT_STATE_CONFIG_ERROR;
    return;
  }

  /* Reset the counter */
  seq_nr_value = 0;

  state = MQTT_CLIENT_STATE_INIT;

  /*
   * Schedule next timer event ASAP
   *
   * If we entered an error state then we won't do anything when it fires.
   *
   * Since the error at this stage is a config error, we will only exit this
   * error state if we get a new config.
   */
  etimer_set(&publish_periodic_timer, 0);

  return;
}
/*---------------------------------------------------------------------------*/
static int
init_config()
{
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));

  memcpy(conf.org_id, DEFAULT_ORG_ID, strlen(DEFAULT_ORG_ID));
  memcpy(conf.type_id, DEFAULT_TYPE_ID, strlen(DEFAULT_TYPE_ID));
  memcpy(conf.auth_token, DEFAULT_AUTH_TOKEN, strlen(DEFAULT_AUTH_TOKEN));
  memcpy(conf.event_type_id, DEFAULT_EVENT_TYPE_ID,
         strlen(DEFAULT_EVENT_TYPE_ID));
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.cmd_type, DEFAULT_SUBSCRIBE_CMD_TYPE, 1);

  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;
  conf.def_rt_ping_interval = DEFAULT_RSSI_MEAS_INTERVAL;
  return 1;
}
/*---------------------------------------------------------------------------*/
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  DBG("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len,
      chunk_len);

//Thomas
printf("Topic-LEN:%i\n",topic_len);
  /* If we don't like the length, ignore */
  if(topic_len != 23 || chunk_len != 1) {
    printf("Incorrect topic or chunk len. Ignored\n");
    return;
  }

  /* If the format != json, ignore */
  if(strncmp(&topic[topic_len - 4], "json", 4) != 0) {
    printf("Incorrect format\n");
  }

  if(strncmp(&topic[10], "leds", 4) == 0) {
    if(chunk[0] == '1') 
    {
      //leds_on(LEDS_RED);
      printf("LED on\n");
# if PROJECT_TARGET_AVR_ZIGBIT
      //LED DS3 on
      PORTB &= ~(1 << PIN5); 
#endif //AN_SOLUTIONS
    } 
    else if(chunk[0] == '0') 
    {
      //leds_off(LEDS_RED);
      printf("LED off\n");
# if PROJECT_TARGET_AVR_ZIGBIT
      PORTB |= (1 << PIN5);
#endif //AN_SOLUTIONS
    }
    return;
  }
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    DBG("APP - Application has a MQTT connection\n");
    timer_set(&connection_life, CONNECTION_STABLE_TIME);
    state = MQTT_CLIENT_STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    DBG("APP - MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    /* Do nothing if the disconnect was the result of an incoming config */
    if(state != MQTT_CLIENT_STATE_NEWCONFIG) {
      state = MQTT_CLIENT_STATE_DISCONNECTED;
      process_poll(&mqtt_client_process);
    }
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    /* Implement first_flag in publish message? */
    if(msg_ptr->first_chunk) {
      msg_ptr->first_chunk = 0;
      DBG("APP - Application received a publish on topic '%s'. Payload "
          "size is %i bytes. Content:\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
    }

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
                msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
    DBG("APP - Application is subscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    DBG("APP - Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    DBG("APP - Publishing complete.\n");
    break;
  }
  default:
    DBG("APP - Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
subscribe(void)
{
  /* Publish MQTT topic in IBM quickstart format */
  mqtt_status_t status;

  status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);

  DBG("APP - Subscribing!\n");
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    DBG("APP - Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
publish(void)
{
  /* Publish MQTT topic in IBM quickstart format */
  DBG("Thomas: publish\n");
  int len;
  int remaining = APP_BUFFER_SIZE;
  char def_rt_str[64];

  //Toggle LED DS2 on MQTT PUBLISH
  PORTB ^= (1 << PIN6); 

  seq_nr_value++;
  buf_ptr = app_buffer;

  len = snprintf(buf_ptr, remaining,
                 "{"
                 "\"d\":{"
                 "\"myName\":\"%s\","
                 "\"Seq #\":%d,"
                 "\"Uptime (sec)\":%lu",
                 BOARD_STRING, seq_nr_value, clock_seconds());

  if(len < 0 || len >= remaining) {
    printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
    return;
  }
  remaining -= len;
  buf_ptr += len;

  /* Put our Default route's string representation in a buffer */
  memset(def_rt_str, 0, sizeof(def_rt_str));
  DBG("Thomas: publish.\n");
 // ipaddr_sprintf(def_rt_str, sizeof(def_rt_str),
 //                                uip_ds6_defrt_choose());
  strncpy(def_rt_str, "Testnachricht", sizeof("Testnachricht"));
  DBG("Thomas: publish..\n");

  
  uint16_t adc_value;
  int batt_level;
  adc_value=adc_read();
  batt_level=(((25400)/1024)*adc_value*2); //Reference Voltage 2.54Volts, 10Bit ADC, 
  //multiplied with ADC Read, multiplied with two because of the 1:1 voltage divider

  len = snprintf(buf_ptr, remaining, ",\"Def Route\":\"%s\",\"Battery (Volt)\":%d",
                 def_rt_str, batt_level);

  //len = snprintf(buf_ptr, remaining, ",\"Def Route\":\"%s\",\"RSSI (dBm)\":%d",
  //               def_rt_str, def_rt_rssi);

  if(len < 0 || len >= remaining) {
    printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
    return;
  }
  remaining -= len;
  buf_ptr += len;

  //memcpy(&def_route, uip_ds6_defrt_choose(), sizeof(uip_ip6addr_t));
  DBG("Thomas: publish...\n");
/*ts
  for(reading = cc26xx_web_demo_sensor_first();
      reading != NULL; reading = reading->next) {
    if(reading->publish && reading->raw != CC26XX_SENSOR_READING_ERROR) {
      len = snprintf(buf_ptr, remaining,
                     ",\"%s (%s)\":%s", reading->descr, reading->units,
                     reading->converted);

      if(len < 0 || len >= remaining) {
        printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
        return;
      }
      remaining -= len;
      buf_ptr += len;
    }
  }
*/
  DBG("Thomas: publish....\n");
  len = snprintf(buf_ptr, remaining, "}}");

  if(len < 0 || len >= remaining) {
    printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
    return;
  }

  mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
               strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);

  DBG("APP - Publish!\n");

}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
               conf.pub_interval * 3);

  state = MQTT_CLIENT_STATE_CONNECTING;
}
/*---------------------------------------------------------------------------*/
//static void
//ping_parent(void)
//{
//  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
//    return;
//  }

//thomas:
//  uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
//uip_ipaddr_t ipaddr;

// uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);

//  uip_icmp6_send(&ipaddr, ICMP6_ECHO_REQUEST, 0,
//                 ECHO_REQ_PAYLOAD_LEN);
//}
/*---------------------------------------------------------------------------*/
static void
state_machine(void)
{
  DBG("state: %i\n",state);
DBG("(MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
  switch(state) {
  case MQTT_CLIENT_STATE_INIT:
    /* If we have just been configured register MQTT connection */
    DBG("MQTT_register\n");
    mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MQTT_CLIENT_MAX_SEGMENT_SIZE);

    /*
     * If we are not using the quickstart service (thus we are an IBM
     * registered device), we need to provide user name and password
     */
    if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) != 0) {
      if(strlen(conf.auth_token) == 0) {
        printf("User name set, but empty auth token\n");
        state = MQTT_CLIENT_STATE_ERROR;
        break;
      } else {
        mqtt_set_username_password(&conn, "use-token-auth",
                                   conf.auth_token);
      }
    }

    /* _register() will set auto_reconnect. We don't want that. */
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    /*
     * Wipe out the default route so we'll republish it every time we switch to
     * a new broker
     */
    memset(&def_route, 0, sizeof(def_route));

    state = MQTT_CLIENT_STATE_REGISTERED;
    DBG("Init\n");
    /* Continue */
  case MQTT_CLIENT_STATE_REGISTERED:
    DBG("Address check\n");
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a public IP. Connect */
      DBG("Registered. Connect attempt %u\n", connect_attempt);
      connect_to_broker();
    }
    etimer_set(&publish_periodic_timer, CC26XX_WEB_DEMO_NET_CONNECT_PERIODIC);
    return;
    break;
  case MQTT_CLIENT_STATE_CONNECTING:
    //leds_on(CC26XX_WEB_DEMO_STATUS_LED);
    //ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    DBG("Connecting (%u)\n", connect_attempt);
    break;
// *Thomas*
  case MQTT_CLIENT_STATE_CONNECTED:
//    /* Don't subscribe unless we are a registered device */
//    //if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) == 0) {
//      DBG("Using 'quickstart': Skipping subscribe\n");
//      state = MQTT_CLIENT_STATE_PUBLISHING;
//    //}
//
    /* Continue */
  case MQTT_CLIENT_STATE_PUBLISHING:
    /* If the timer expired, the connection is stable. */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
      connect_attempt = 0;
    }

    if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      /* Connected. Publish */
      if(state == MQTT_CLIENT_STATE_CONNECTED) {
        subscribe();
        state = MQTT_CLIENT_STATE_PUBLISHING;
      } else {
       // leds_on(CC26XX_WEB_DEMO_STATUS_LED);
       // ctimer_set(&ct, PUBLISH_LED_ON_DURATION, publish_led_off, NULL);
        publish();
      }
      etimer_set(&publish_periodic_timer, conf.pub_interval);

      DBG("Publishing\n");
      /* Return here so we don't end up rescheduling the timer */
      return;
    } else {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd).
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us.
       */
      DBG("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
          conn.out_queue_full);
    }
    break;
  case MQTT_CLIENT_STATE_DISCONNECTED:
    DBG("Disconnected\n");
    if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
      mqtt_disconnect(&conn);
      connect_attempt++;

      interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
        RECONNECT_INTERVAL << 3;

      DBG("Disconnected. Attempt %u in %lu ticks\n", connect_attempt, interval);

      etimer_set(&publish_periodic_timer, interval);

      state = MQTT_CLIENT_STATE_REGISTERED;
      return;
    } else {
      /* Max reconnect attempts reached. Enter error state */
      state = MQTT_CLIENT_STATE_ERROR;
      DBG("Aborting connection after %u attempts\n", connect_attempt - 1);
    }
    break;
  case MQTT_CLIENT_STATE_NEWCONFIG:
    /* Only update config after we have disconnected */
    if(conn.state == MQTT_CONN_STATE_NOT_CONNECTED) {
      update_config();
      DBG("New config\n");

      /* update_config() scheduled next pass. Return */
      return;
    }
    break;
  case MQTT_CLIENT_STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
    printf("Bad configuration.\n");
    return;
  case MQTT_CLIENT_STATE_ERROR:
  default:
    //leds_on(CC26XX_WEB_DEMO_STATUS_LED);
    /*
     * 'default' should never happen.
     *
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
    printf("Default case: State=0x%02x\n", state);
    return;
  }

  /* If we didn't return so far, reschedule ourselves */
  etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
}

ISR(INT6_vect)
{            
  if (button_flag == 0)
  {
      button_flag = 1; //button pressed
      //PORTB ^= (1 << PIN5);

      process_poll(&mqtt_client_process);
  }
  //printf("interrupt was triggered on INT0... \n");  
}


/*---------------------------------------------------------------------------*/
AUTOSTART_PROCESSES(&mqtt_client_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_client_process, ev, data)
{
  PROCESS_BEGIN();
    uip_ip6addr_t ipaddr;
    uip_ip6addr(&ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1);
//  uip_netif_addr_autoconf_set(&ipaddr, &uip_lladdr);
//  uip_netif_addr_add(&ipaddr, 16, 0, TENTATIVE);
  //  uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);

  printf("\n\nSimple MQTT service startet...\n");
  printf("Build --- %s\n",__DATE__); /*Compile-Datum*/
  printf("      --- %s\n",__TIME__); /*Compile-Zeit*/
  printf("      --- %s\n",__FILE__); /*Compile-Datei*/
 
  printf("      --- " CONTIKI_VERSION_STRING "\n");
  printf("\nUIP_APPDATA: %u\nUIP_CONF_BUFFER: %u\n", UIP_APPDATA_SIZE, UIP_CONF_BUFFER_SIZE);
 
//#if MINIMAL_NET != 1
# ifndef PROJECT_TARGET_MINIMAL_NET
  printf("      --- " BOARD_STRING "\n");
  printf(" Net: ");
  printf("%s\n", NETSTACK_NETWORK.name);
  printf(" MAC: ");
  printf("%s\n", NETSTACK_MAC.name);
  printf(" RDC: ");
  printf("%s", NETSTACK_RDC.name);
#endif /* minimal-net */

//#if AN_SOLUTIONS == 1
# if PROJECT_TARGET_AVR_ZIGBIT
	/*Set LEDs as Output and switch LED DS3 on @ANY Brick*/
	DDRB |= (1 << PIN5);
	DDRB |= (1 << PIN6);
	DDRB |= (1 << PIN7);
	PORTB &= ~(1 << PIN7); /*DS3: LOW  -> on: Indicate Operation*/
	PORTB |= (1 << PIN6);  /*DS2: HIGH -> off: MQTT Ops*/
	PORTB |= (1 << PIN5);  /*DS1: HIGH -> off: MQTT Configurable */
	/*END LEDs @ANY Brick*/
        
        /*Enable IRQ6 (Pushbutton)*/
	DDRE &= ~(1 << DDB6);   /* PE6 as input */
        PORTE |= (1 << PB6);   /* enable internal pull-up on PE6 */
        EIMSK &= ~(1 << INT6); //disable interrupts before changing EICRA
        EICRB &= ~(1 << ISC60); //EICRB 00xx|0000 low-level triggers interrupt on int6 (p. 111)
        EICRB &= ~(1 << ISC61); //EICRB 00xx|0000 low-level triggers interrupt on int6 (p. 111)
        EIMSK |= (1 << INT6); // enable INT6 (datasheet p. 219 ff)
	/*End enable IRQ6 (Pushbutton)*/

	/*Enable ADC measurements (voltage level)*/
	adc_init();
#endif //AN_SOLUTIONS


//# if MINIMAL_NET == 1
# if PROJECT_TARGET_MINIMAL_NET
  printf("\nPress any key to continue!\n");
  getchar();
# endif /*MINIMAL_NET*/

  /*
   * Setting up a listening MQTT Client.
   */
  if(init_config() != 1) {
    printf("init_config error!\n");
    PROCESS_EXIT();
  }

   DBG("config done, now register\n"); 
   mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MQTT_CLIENT_MAX_SEGMENT_SIZE);

   update_config();

  //uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
  //                                  echo_reply_handler);
  //etimer_set(&echo_request_timer, conf.def_rt_ping_interval);

  static struct etimer debounce_timer;
  static uint8_t debounce_state = 1;
  

  /*
   * Loop for ever, accepting new connections.
   */
  DBG("entering main-loop\n");
  while(1) 
  {
    /*
     * Blocks until we get the first TCP/IP event.
     */
    DBG("Sleeping...\n");
    PROCESS_YIELD();

    DBG("Waking...\n");
 	if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
 	    ev == PROCESS_EVENT_POLL || (ev == PROCESS_EVENT_TIMER && data == &debounce_timer)) 
	{
//      printf("timer-event\n");

	if (data == &debounce_timer)
	{
		button_flag = 0;
		debounce_state = 1;
	}
  	else if (button_flag == 1 && debounce_state == 1)
  	{
//		if (debounce_state == 1)
//		{
			debounce_state = 0;			
			PORTB ^= (1 << PIN5); 
			etimer_set(&debounce_timer, CLOCK_SECOND/5);
//		}
        }
        else
	{
       		state_machine();
	}
     }
    
/* eliminated the regular PING    
    if(ev == PROCESS_EVENT_TIMER && data == &echo_request_timer) 
    {
DBG("ping\n");
//#if AN_SOLUTIONS == 1
# if PROJECT_TARGET_AVR_ZIGBIT
      //Blink LED DS2 on PING
      PORTB &= ~(1 << PIN6); 
#endif //AN_SOLUTIONS
      ping_parent();
//#if AN_SOLUTIONS == 1
# if PROJECT_TARGET_AVR_ZIGBIT
      PORTB |= (1 << PIN6);
#endif //AN_SOLUTIONS
      etimer_set(&echo_request_timer, conf.def_rt_ping_interval);
    }
*/
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/