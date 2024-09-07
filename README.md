# FOR ME AND JONO

Dude add to this where I'm missing stuff because I 100% am

## To be implemented:

### Networking features
- need to connect two clients to one another
- a way to host IP address based on username? 
- connections might only be usable when two clients are connected 
  #### Key components: 
  - get user global IP address
  - open port to listen for requests
  - lookup other user's IP address 
  - use internet to connect to desired IP address
  - allow for messages to be sent
  - receive messages and display on screen with timestamps
  - allow for images? hosted on a server or sent byte by byte (wont be displayable in console, maybe downloaded)


### User interface
- using win32 API
- should be finished* after network connectivity - application might be difficult to test without 




### Data storage?
- hopefully immutable/encrypted files
- option to save chat ? 


***

### IDEAS!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 

- send packets with a header detailing message info (ex: length, time sent, data type ) will need to be parsed by the recipient 

- packets will be a stream of bytes (first byte an int with message length, second byte a date/unix epoch time)

- use an enum for each message packet (one for incoming message, one for invitation, user logoff)

- allocate memory based on packet length first then create struct with char length of that size (in bytes)
**- message struct will have a char message[] variable in the last field, and malloc() used before init

- server on a raspberry pi ?? running sqlite containing usernames and current IP address ? maybe an active status too

- multiple threads for maintaining connection, waiting for message streams (c11 needed )
