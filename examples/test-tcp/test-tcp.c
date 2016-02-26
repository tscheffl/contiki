/*
 * \file
 *         A simple Contiki application for testing TCP connections
 * \author
 *         Thomas Scheffler <scheffler@beuth-hochschule.de>
 */

#include "contiki.h"
#include "contiki-net.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ECHO 0x65 /* echo mode */
#define BLCK 0x62 /* black-hole mode */
#define SEND 0x73 /* send mode */
#define EOT  0x71 /* end of transmission */

#define BUFFER_SIZE 13

static struct psock ps;
static uint8_t buffer[BUFFER_SIZE];
static uint8_t send_buffer[80];

static int bytes_to_send = 0; /* Number of bytes to send */
static int temp;

/*---------------------------------------------------------------------------*/
static unsigned short
generate_data(void *count)
{
  uint8_t i;
  printf("sending Byte#: %i\n", *(int *)count);
  for(i = 0; !((i >= UIP_APPDATA_SIZE) || (i >= *(int *)count)); i++) {
    *((char *)uip_appdata + i) = 'A';
  }

  return i;
}
/*---------------------------------------------------------------------------*/

static PT_THREAD(handle_connection(struct psock *p))
{
  PSOCK_BEGIN(p);
  /*
   * Send a welcome message using PSOCK_SEND_STR(),
   * that sends a null-terminated string.
   */
  PSOCK_SEND_STR(p, "\nSimple TCP-test service\n");
  sprintf((char *)send_buffer, "BUFFER_SIZE: %u\nUIP_APPDATA: %u\nUIP_CONF_BUFFER: %u\n", BUFFER_SIZE, UIP_APPDATA_SIZE, UIP_CONF_BUFFER_SIZE);
  PSOCK_SEND_STR(p, (char *)send_buffer);

  /*
   * The PSOCK_READTO() function reads incoming data
   * from the TCP connection until it gets a newline character.
   * The number of read bytes depents on the length
   * of the input buffer. The rest of the line, up to
   * the newline is discarded. It is however still in the
   * UIP_BUFFER.
   */
  PSOCK_READTO(p, '\n');

  /* ---------------------------------------------------------
   * ECHO routine immediately sends back all received content.
   * ---------------------------------------------------------
   */

  if(buffer[0] == ECHO) {
    while(1) {
/*      PSOCK_READTO(p, '\n'); */
      PSOCK_READBUF(p);
      if(buffer[0] == EOT) {
        PSOCK_SEND_STR(p, "Leaving ECHO routine!\r\n");
        buffer[0] = 0; /* cancel last command */
        goto END_ECHO;
      }
      /*
       * Echo the received content back. PSOCK_DATALEN()
       * provides the length of the received data.
       * This length will not be longer than the input buffer:
       * UIP_CONF_BUFFER_SIZE
       */
      printf("received Byte#: %i \nreceived String: %s\n", PSOCK_DATALEN(p), buffer);
      PSOCK_SEND(p, buffer, PSOCK_DATALEN(p));
    } /* while */
END_ECHO:;
  } /* if ECHO */

  /* ---------------------------------------------------------
   * BLACK-hole routine sucks in all received content, without
   * sending anything back (just ACK-ing the reception).
   * ---------------------------------------------------------
   */

  if(buffer[0] == BLCK) {
    while(1) {
/*      PSOCK_READTO(p, '\n'); */
      PSOCK_READBUF(p);

      if(buffer[0] == EOT) {
        PSOCK_SEND_STR(p, "Leaving BLACK-hole routine!\r\n");
        buffer[0] = 0; /* cancel last command */
        goto END_BLCK;
      }
      /*
       * Sink the received content. PSOCK_DATALEN()
       * provides the length of the received data.
       */
      printf("received Byte#: %i\n", PSOCK_DATALEN(p));
    } /* while */
END_BLCK:;
  } /* if ECHO */

  /* ---------------------------------------------------------
   * SEND routine expects to receive a NUMBER and immediately
   * sends back [NUMBER * A] via the connection, generates
   * multiple IP-packets if necessary.
   * ---------------------------------------------------------
   */

  if(buffer[0] == SEND) {
    while(1) {
/*      PSOCK_READTO(p, '\n'); */
      PSOCK_READBUF(p);

      if(buffer[0] == EOT) {
        PSOCK_SEND_STR(p, "Leaving SEND routine!\r\n");
        buffer[0] = 0; /* cancel last command */
        goto END_SEND;
      }
      /*
       * Send back n times A
       */
      bytes_to_send = atoi((char *)buffer);

      /* Send one Buffer full of A */
      /*
         printf("received: %i\n",bytes_to_send);
         if (bytes_to_send > BUFFER_SIZE - 1) bytes_to_send = BUFFER_SIZE - 1;
         printf("converted: %i\n",bytes_to_send);

         memset(buffer,'A',bytes_to_send);
         PSOCK_SEND(p, buffer, bytes_to_send);
       */

      /*Break data into multiple transmissions*/
      while(bytes_to_send > UIP_APPDATA_SIZE) {
        temp = UIP_APPDATA_SIZE;
        PSOCK_GENERATOR_SEND(p, generate_data, &temp);
        bytes_to_send = bytes_to_send - UIP_APPDATA_SIZE;
      }
      PSOCK_GENERATOR_SEND(p, generate_data, &bytes_to_send);
    } /* while */
END_SEND:;
  }

  if(buffer[0] == EOT) {
    PSOCK_SEND_STR(p, "Good bye!\r\n");
    PSOCK_CLOSE(p);
  }

  PSOCK_END(p);
}

/*---------------------------------------------------------------------------*/
PROCESS(tcp_test_process, "TCP-Test process");
AUTOSTART_PROCESSES(&tcp_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcp_test_process, ev, data)
{
  PROCESS_BEGIN();

  printf("\n\nSimple TCP-test service startet...\n");
  /*
   * Setting up a listening TCP port.
   * The UIP_HTONS() macro converts the port number to
   * network byte order.
   */
  tcp_listen(UIP_HTONS(49999));
  /*
   * Loop for ever, accepting new connections.
   */
  while(1) {
    /*
     * Blocks until we get the first TCP/IP event.
     */
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    /*
     * If a client connected, the protosocket will be initialized.
     */
    if(uip_connected()) {
      /*
       * PSOCK_INIT() initializes the protosocket and
       * binds the input buffer to the protosocket.
       */
      PSOCK_INIT(&ps, buffer, BUFFER_SIZE);
      /*
       * Loops until the connection is aborted, closed, or times out.
       */
      while(!(uip_aborted() || uip_closed() || uip_timedout())) {
        /*
         * Wait for a TCP/IP event. So other processes can
         * run while our process is waiting.
         */
        PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
        /*
         * Here is where the action is: handle_connection()
         * uses the defined protosocket to communicate with the client.
         */
        handle_connection(&ps);
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
