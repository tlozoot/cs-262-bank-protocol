Wire protocol for a simple banking application
=============================================

**Student ID: 136**  
**CS 262, Spring 2012**  
**2/7/2012**


Preliminaries
=============

This protocol has been designed as a concise way of implementing several simple
banking transactions. Request and response messages are designed to be very
small. In some cases, however, longer messages than strictly necessary are used
to make processing simpler. The protocol is designed so that additional methods
and exceptions can be easily added, and existing ones can be modified with
additional fields or comments in future versions.

Much of the framework code was provided is borrowed from the
[support code](http://csapp.cs.cmu.edu/public/code.html) from Bryant and
O'Hallaron's _Computer Systems: A Programmer's Perspective_.


Protocol messages
=================

Protocol messages have the following format:

1. Version [1 byte]

2. Operation code [1 byte]

3. Payload [size determined by operation code]


These messages differ from the
[SGS protocol](http://www.reddwarfserver.org/javadoc/current/server-api/) in
two important ways:

1.  In the SGS protocol, clients initiate a session which contains a version
    number. In this simpler protocol, every pair of messages sent and received
    constitutes an implicit session, and so the version number must be included
    at the start of every message. Although technically server responses don't
    need a version, maintaining two sets of message formats seems rather
    complicated when only one byte is at stake.

2.  Payload size isn't a necessary part of this protocol, because the payload
    size can be determined from the op code (as with CPU instructions).
    Payloads only vary in size when they contain a string or byte array, and
    those types are responsible for encoding how long they are (see below). If
    a payload consists of multiple arguments, then they are simply concatenated
    with no additional overhead. 


Data types
==========

amt
---

Monetary sums are given the data type `amt`. An amount corresponds to
hundredths of a dollar (aka pennies) and is represented by an unsigned long
long integer (8 bytes). This is a safe choice because we can assume, barring
the advent of hyperinflation, that banking clients are maintaining balances of
less than USD 264 / 100.

acct
----

Account numbers are given the data type `acct`. They are represented by an
unsigned long integer (4 bytes). We will assume that the bank does not have
more than 2^32 clients.

byte[]
------

Byte arrays begin with an unsigned short integer (2 bytes) representing its
size (not including the value of the size itself). A byte array may be empty,
in which case the entire byte array would be encoded as `0x0000`.
Transmitting the length is necessary, because byte arrays (and strings) are the
only variable size elements of a payload. Byte arrays are useful in exceptions
for returning the message that generated them.

string
------

Strings are byte arrays with a UTF-8 encoded payload. Thus they begin with an
unsigned short integer size, and the empty string would be encoded as `0x0000`. 

Methods
=======

accountNumber createAccount(initialDepositAmount);
--------------------------------------------------

A client requests to open an account with `initialDepositAmount` cents. The server responds with the ID of the newly created account if the operation is successful.

**Client request**  
_Opcode_: `0x10`  
_Payload_: 1. `(amt) initialDepositAmount`

**Server response (success)**  
_Opcode_: `0x11`  
_Payload_: 1. `(acct) accountNumber`

balance deposit(acctNumber, amount);
------------------------------------

A client requests to deposit amount cents into the account represented by acctNumber. The server responds with account's current balance if it the operation successful. If the specified account does not exist, the server will return a “no such account” exception (see below).
Client request
Opcode: 0x20
Payload: 1. (acct) acctNumber, 2. (amt) amount
Server response (success)
Opcode: 0x21
Payload: 1. (amt) balance
balance withdraw(acctNumber, amount);
A client requests to withdraw amount cents into the account represented by acctNumber. If the client has more than amount in his account, and the operation is successful, amount will be deducted from his account and the account balance returned. If the client does not have enough money, than an “insufficient funds” exception will be returned. If the specified account does not exist, the server will return a “no such account” exception (see below).
Client request
Opcode: 0x30
Payload: 1. (acct) acctNumber, 2. (amt) amount
Server response (success)
Opcode: 0x31
Payload: 1. (amt) balance
balance getBalance(acctNumber);
A client requests to deposit amount cents into the account represented by acctNumber. The server responds with account's current balance if it the operation successful.  If the specified account does not exist, the server will return a “no such account” exception (see below).
Client request
Opcode: 0x40
Payload: 1. (acct) acctNumber, 2. (amt) amount
Server response (success)
Opcode: 0x41
Payload: 1. (amt) balance
acctStatus close(acctNumber);
A client makes a request to close acctNumber. If the operation is successful, the server will return a success response code with no payload.  If the specified account does not exist, the server will return a “no such account” exception. If the operation fails, a general “internal server error” exception will be returned (see below).
Client request
Opcode: 0x50
Payload: 1. (acct) acctNumber
Server response (success)
Opcode: 0x51
Payload: [none]


Exceptions
Exceptions return the request that generated them as payload, along with an optional error message (they may just use the empty string). Returning the original request is helpful in debugging, when there could be many clients and many messages, and it is not clear which messages are generating which exceptions. Including an error string with an exception may be useful for giving more information about a general server error, or suggesting which versions or operation codes are available.
Unknown version
The server does not recognize the version of a message and cannot process it. The server returns the version that it parsed from the request.
Opcode: 0x90
Payload: 1. (byte[]) request, 2. (byte[]) requestVersion, 3. (string) errorMessage 
Unknown operation code
The server does not recognize the operation code of a message and cannot process it. The server returns the operation code that it parsed from the request.
Opcode: 0x91
Payload: 1. (byte[]) request, 2. (byte[]) requestOpCode, 3. (string) errorMessage
Invalid payload
The server doesn't know how to interpret the payload of a message. This could occur, for example, if a payload is the wrong size, or if a string is formatted incorrectly. The server returns the payload that it parsed from the request.
Opcode: 0x92
Payload: 1. (byte[]) request, 2. (byte[]) requestPayload, 2. (string) errorMessage
Insufficient funds
The client tried to withdraw more money than is in the given account. In that case, no money will be deducted from the account and the original account balance will be returned to the client in the payload.


Opcode: 0x93
Payload: 1. (byte[]) request, 2. (acct) accountNumber, 3. (amt) balance, 4. (string) errorMessage
No such account
The client requested an operation on an account that doesn't exist.


Opcode: 0x94
Payload: 1. (byte[]) request, 2. (acct) accountNumber, 3. (string) errorMessage
Request denied
The server decides not to process the request due to detection of some kind of fraud or abuse by a client. This could arise, for example, when too many accounts are created too quickly from the same client, or if too much money is deposited in one account.


Opcode: 0x95
Payload: 1. (byte[]) request, 2. (string) errorMessage
Internal server error
Equivalent to an HTTP 500 response; a catch-all exception for when an arbitrary error occurs on the server while processing a request. 
Opcode: 0x96
Payload: 1. (byte[]) request, 2. (string) errorMessage
