# CallerInfo protocol version 3.0.0 #
## General information ##
Version 3.0.0 of this protocol was created to have a more robust and flexible way
to let CallerInfo clients and server communicate with each other. Messages are
transformed to JSON objects and sent as a string.

Current applications using this protocol are fritz2cid as a server and ci3 (ciclient)
and ciservice3 as clients.

### Basic form of communication ###
After connecting to the server the client should send a `CI_NET_MSG_VERSION`
indicating the supported protocol version. The server should reply to this
with the protocol version the server supports. No incompatible messages
should be sent from either side to allow backward compatibility.
Currently only version 3.0.0 exists.

`RING` and `CALL` messages are only sent by the server. Unhandled messages should be
ignored. A server should reply to all DB messages with the same message type
even if the action cannot be performed to allow the client to handle this appropriately.
A guid is intended as a internal id for the client and should be used in a reply as-is.
This allows the client to handle replies for different messages different.

Multipart messages consist of at least two parts: Init and Complete. Both should be sent.
In between Update messages may be sent.

### Format of a message ###
Messages are sent as a byte stream. Each message starts with a six byte magic string "**`ci-msg`**"
followed by four bytes indicating the size of the payload in bytes, least significant byte first,
and four bytes of the message type, least significant byte first.
After that the payload follows. This is a string describing a JSON object.

## Messages ##

### General data ###
All messages contain a member **`guid`** (_`int`_) with an unsigned integer. This may be set by the
querying end and passed back in the reply.

Multipart messages contain the following members:

 * **`stage`**: (_`int`_) An integer with the current stage of the message:
     + 0: `MultipartStageInit`, first part of the message
     + 1: `MultipartStageUpdate`, updated information of the message. This stage may be sent multiple
          times or never.
     + 2: `MultipartStageComplete`, last part of the message.
 * **`part`**:  (_`int`_) An integer with the numbered part of the message, currently not used.
 * **`msgid`**: (_`string`_) A short null-terminated string of at most 15 characters plus a null-byte to indicate
                related messages. This must be the same in all stages.

The `CICallInfo` and `CICallerInfo` objects may be part of multiple messages and may be
embedded in them, i.e. the members of these objects occur in the message content and there is
no extra level of containers.

#### `CICallInfo` ####
This object contains all information about a call.

 * **`id`**: (_`int`_)
 * **`completenumber`**: (_`string`_)
 * **`areacode`**: (_`string`_)
 * **`number`**: (_`string`_)
 * **`date`**: (_`string`_)
 * **`time`**: (_`string`_)
 * **`msn`**: (_`string`_)
 * **`alias`**: (_`string`_)
 * **`area`**: (_`string`_)
 * **`name`**: (_`string`_)

#### `CICallerInfo` ####
This object contains all information about a caller.

 * **`number`**: (_`string`_)
 * **`name`**: (_`string`_)

### `VERSION` (0) ###
 * **`major`**: (_`int`_)
 * **`minor`**: (_`int`_)
 * **`patch`**: (_`int`_)
 * **`human_readable`**: (_`string`_)

### `EVENT_RING` (1) ###
 * *Multipart message*
 * `CICallInfo`: (_embedded_)

### `EVENT_CALL` (2) ###
 * `CICallInfo`: (_embedded_)

### `EVENT_CONNECT` (3) ###
 _Currently not implemented._

### `EVENT_DISCONNECT` (4) ###
 _Currently not implemented._

### `LEAVE` (5) ###
 No additional data.

### `SHUTDOWN` (6) ###
 No additional data.

### `DB_NUM_CALLS` (7) ###
 * **`count`**: (_`int`_)

### `DB_CALL_LIST` (8) ###
 * **`user`**: (_`int`_)
 * **`offset`**: (_`int`_)
 * **`count`**: (_`int`_)
 * **`calls`**: Array of `CICallInfo` objects.

### `DB_GET_CALLER` (9) ###
 * **`user`**: (_`int`_)
 * `CICallerInfo`: (_embedded_)

### `DB_ADD_CALLER` (10) ###
 * **`user`**: (_`int`_)
 * `CICallerInfo`: (_embedded_)

### `DB_DEL_CALLER` (11) ###
 * **`user`**: (_`int`_)
 * `CICallerInfo`: (_embedded_)

### `DB_GET_CALLER_LIST` (12) ###
 * **`user`**: (_`int`_)
 * **`filter`**: (_`string`_)
 * **`callers`**: Array of `CICallerInfo` objects.
