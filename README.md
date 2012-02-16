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

3. Account number [4 bytes]

4. Amount [8 bytes]

5. Error string [1 - 64k bytes]

Although all messages have this format, not every field will be defined in
every message. For example, the account number is undefined when opening an
account. Error strings are always optional. No message will be more than 
65535 bytes.


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
    Payloads only vary in size when they contain a string, and
    those types are responsible for encoding how long they are (see below).


Data types
==========

acct
----

Account numbers are given the data type `acct`. They are represented by an
unsigned integer (4 bytes). We will assume that the bank does not have
more than 2^32 clients.

amt
---

Monetary sums are given the data type `amt`. An amount corresponds to
hundredths of a dollar (aka pennies) and is represented by an unsigned long
long integer (8 bytes). This is a safe choice because we can assume, barring
the advent of hyperinflation, that banking clients are maintaining balances of
less than USD 264 / 100.


string
------

Strings are ASCII encoded and null-terminated. They may consist of just the
null byte `0x0000`. No string will exceed 65519 bytes in length.

Methods
=======

accountNumber createAccount(initialDepositAmount);
--------------------------------------------------

A client requests to open an account with `initialDepositAmount` cents. The
server responds with the ID of the newly created account if the operation is
successful.

**Client request**  
_Opcode_: `0x10`  
_Account_: (undefined)
_Amount_: `initialDepositAmount`

**Server response (success)**  
_Opcode_: `0x11`  
_Account_: `accountNumber`
_Amount_: `initialDepositAmount`


balance deposit(acctNumber, amount);
------------------------------------

A client requests to deposit amount cents into the account represented by
`acctNumber`. The server responds with account's current balance if the
operation is successful. If the specified account does not exist, the server
will return a "no such account" exception (see below).

**Client request**  
_Opcode_: `0x20`  
_Account_: `acctNumber`,
_Amount_: `amount`

**Server response (success)**  
_Opcode_: `0x21`  
_Account_: `acctNumber`
_Amount_: `balance`


balance withdraw(acctNumber, amount);
-------------------------------------

A client requests to withdraw amount cents into the account represented by
`acctNumber`. If the client has more than amount in his account, and the
operation is successful, amount will be deducted from his account and the
account balance returned. If the client does not have enough money, than an
"insufficient funds" exception will be returned. If the specified account does
not exist, the server will return a "no such account" exception (see below).

**Client request**  
_Opcode_: `0x30`  
_Account_: `acctNumber`,
_Amount_: `amount`

**Server response (success)**  
_Opcode_: `0x31`  
_Account_: `acctNumber`
_Amount_: `balance`


balance getBalance(acctNumber);
-------------------------------

A client requests to deposit amount cents into the account represented by
`acctNumber`. The server responds with account's current balance if it the
operation successful.  If the specified account does not exist, the server
will return a "no such account" exception (see below).

**Client request**  
_Opcode_: `0x40`  
_Account_: `acctNumber`,
_Amount_: (undefined)

**Server response (success)**  
_Opcode_: `0x41`  
_Account_: `acctNumber`
_Amount_: `balance`


acctStatus close(acctNumber);
-----------------------------

A client makes a request to close `acctNumber`. If the operation is successful,
the server will return a success response code with no payload.  If the
specified account does not exist, the server will return a "no such account"
exception. If the operation fails, a general "internal server error" exception
will be returned (see below).

**Client request**  
_Opcode_: `0x50`  
_Account_: `acctNumber`,
_Amount_: (undefined)

**Server response (success)**  
_Opcode_: `0x51`  
_Account_: `acctNumber`
_Amount_: (undefined)


Exceptions
==========

Exceptions will use the amount and account fields where relevant.  For most
exceptions they will be undefined.  Exceptions may also use the error string
to return additional info that could be helpful in debugging, such as echoing
which opcode or version was received, or which are available.


Unknown version
---------------

_Opcode_: `0x90`  

The server does not recognize the version of a message and cannot process it.
The server returns the version that it parsed from the request.


Unknown operation code
----------------------

_Opcode_: `0x91`  

The server does not recognize the operation code of a message and cannot
process it. The server returns the operation code that it parsed from the
request.


Invalid payload
---------------

_Opcode_: `0x92`  

The server doesn't know how to interpret the payload of a message. This could
occur, for example, if a payload is the wrong size, or if a string is formatted
incorrectly. The server returns the payload that it parsed from the request.


Insufficient funds
------------------

_Opcode_: `0x93`

The client tried to withdraw more money than is in the given account. In that
case, no money will be deducted from the account and the original account
balance will be returned to the client in the payload.



No such account
---------------

_Opcode_: `0x94`  

The client requested an operation on an account that doesn't exist.


Too many accounts
-----------------

_Opcode_: `0x95`  

The client tried to create an account, but too many accounts already exist.


Request denied
--------------

_Opcode_: `0x96`  

The server decides not to process the request due to detection of some kind of
fraud or abuse by a client. This could arise, for example, when too many
accounts are created too quickly from the same client, or if too much money is
deposited in one account.


Internal server error
---------------------

_Opcode_: `0x97`  

Equivalent to an HTTP 500 response; a catch-all exception for when an arbitrary
error occurs on the server while processing a request.
