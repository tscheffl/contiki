# Simple TCP test server

This is a simple tcp server written for the purpose of testing and benchmarking the network performance of Contiki.
When the server is started it sends the current settings for BUFFER_SIZE, UIP_APPDATA_SIZE and UIP_CONF_BUFFER_SIZE back
to the client.

The server provides 3 modes that can be activated at any time by sending certain command letters to the server.
The modes (and the connection itself) can be quitted by sending the letter '***q***':

- **Black-hole** mode 
- **Send** or generator mode
- **Echo** mode

###  **Black-hole** mode 
The server sucks in all data that is send to it and does nothing, except ACK-ing the reception of the packets. 
- This mode can be activated by sending the letter '***b***' to the server.

###  **Send** (or generator) mode 
The server generates a certain number of bytes (multiples of the letter 'A') and sends them back to the client. The server expects to receive a decimal number (formated as ASCII string) from the client, to determine how many bytes should be send back. 
- This mode is activated by sending the letter '***s***' followed by a new packet containing a decimal number.
- Payload from the server will be chunked by UIP_APPDATA_SIZE

###  **Echo** mode 
The server echos back all the bytes that have been received from the client. 
- This mode is activated by sending the letter '***e***', followed by a new packet with payload that will be echoed back.
- Payload from the server will be chunked by BUFFER_SIZE

###Attention!
The server currently looks for the letter '***q***' in the first byte of any received packet. So, in order to get robust results, you better make sure that the data send to the server does not contain this letter.
Alternatively you can alter the code to support only one mode at a time, but this might make testing a bit more cumbersome... 
