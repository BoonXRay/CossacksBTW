/*
GameSpy Chat SDK 
Dan "Mr. Pants" Schoenblum
dan@gamespy.com

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com
*/

/*************
** INCLUDES **
*************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Chat.h"
#include "ChatMain.h"
#include "ChatSocket.h"
#include "ChatHandlers.h"
#include "ChatChannel.h"
#include "ChatCallbacks.h"
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__)  && !defined(__mips64)
	#include "::nonport.h"
#else
    #include "nonport.h"
#endif

/************
** DEFINES **
************/
#define CI_DO_BLOCKING      if(blocking)\
							{\
								do{\
									ciThink(chat, ID);\
                                    Sleep(10);\
								}while(ciCheckForID(chat, ID));\
							}

#define ASSERT_CHANNEL()    assert(channel != NULL); assert(channel[0] != '\0');
#define ASSERT_NICK()       assert(nick != NULL); assert(nick[0] != '\0'); assert(strlen(nick) < MAX_NICK);
#define ASSERT_USER(user)   assert(user != NULL); assert(user[0] != '\0'); assert(strlen(user) < MAX_USER);
#define ASSERT_MESSAGE()    assert(message != NULL); assert(message[0] != '\0');
#define ASSERT_TYPE(type)   assert((type == CHAT_MESSAGE) || (type == CHAT_ACTION) || (type == CHAT_NOTICE) || (type == CHAT_UTM));
#define ASSERT_PASSWORD()   assert(password != NULL); assert(password[0] != '\0');
#define ASSERT_BAN()        assert(ban != NULL); assert(ban [0] != '\0');

/**********
** TYPES **
**********/

typedef struct ciEnumUsersData
{
	chatEnumUsersCallback callback;
	void * param;
} ciEnumUsersData;

/**************
** FUNCTIONS **
**************/
static CHATBool ciProcessServerMessage(CHAT chat, ciServerMessage * message)
{
	int i;

	assert(message != NULL);

	// Figure out what type of message this is.
	///////////////////////////////////////////
	for(i = 0 ; i < numServerMessageTypes ; i++)
	{
		// Does the type match?
		///////////////////////
		if(strcasecmp(message->command, serverMessageTypes[i].command) == 0)
		{
			// Is there a handler?
			//////////////////////
			if(serverMessageTypes[i].handler != NULL)
			{
				// Call the handler.
				////////////////////
				serverMessageTypes[i].handler(chat, message);
			}

			return CHATTrue;
		}
	}

	// Didn't find a match.
	///////////////////////
	return CHATFalse;  //ERRCON
}

static CHATBool ciCheckForID(CHAT chat, int ID)
{
	return (ciCheckFiltersForID(chat, ID) || ciCheckCallbacksForID(chat, ID));
}

void ciHandleDisconnect(CHAT chat, const char * reason)
{
	CONNECTION;

	// Check if we've already handled this.
	///////////////////////////////////////
	if(connection->disconnected)
		return;

	// Not connected anymore.
	/////////////////////////
	connection->connected = CHATFalse;
	connection->connecting = CHATFalse;
	connection->disconnected = CHATTrue;

	// Call the disconnected callback.
	//////////////////////////////////
	if(connection->globalCallbacks.disconnected != NULL)
	{
		ciCallbackDisconnectedParams params;
		params.reason = (char *)reason;
		ciAddCallback(chat, CALLBACK_DISCONNECTED, connection->globalCallbacks.disconnected, &params, connection->globalCallbacks.param, 0, NULL);
	}
}

static void ciThink(CHAT chat, int ID)
{
	ciServerMessage * message;
	CONNECTION;

	// Is the socket connected?
	///////////////////////////
	if(connection->chatSocket.connectState == ciConnected)
	{
		// Do processing.
		/////////////////
		ciSocketThink(&connection->chatSocket);

		// Have we lost connection?
		///////////////////////////
		if(connection->chatSocket.connectState == ciDisconnected)
		{
			ciHandleDisconnect(chat, "Disconnected");
		}
		else
		{
			// Check received messages.
			///////////////////////////
			while((message = ciSocketRecv(&connection->chatSocket)) != NULL)
			{
				// Call the raw callback.
				/////////////////////////
				if(connection->globalCallbacks.raw != NULL)
				{
					ciCallbackRawParams params;
					params.raw = message->message;
					ciAddCallback(chat, CALLBACK_RAW, connection->globalCallbacks.raw, &params, connection->globalCallbacks.param, 0, NULL);
				}

				// Process the message.
				///////////////////////
				ciProcessServerMessage(chat, message);
			}
		}
	}
		
	// Let the filters think.
	/////////////////////////
	ciFilterThink(chat);

	// Call callbacks.
	//////////////////
	ciCallCallbacks(chat, ID);
}

/************
** GENERAL **
************/
void ciSendNickAndUser(CHAT chat)
{
	char * sendingNick;
	char * str;
	CHATBool invalidNick;

	CONNECTION;

	// Check for an invalid nick.
	/////////////////////////////
	invalidNick = CHATFalse;
	if(strchr("0123456789", connection->nick[0]))
		invalidNick = CHATTrue;
	for(str = connection->nick ; str[0] ; str++)
	{
		if(!strchr(VALID_NICK_CHARS, str[0]))
			invalidNick = CHATTrue;
	}

	// The IRC server doesn't always reject bad nicks, sometimes it silently changes them.
	// So if we have something we know is invalid, send something we know the server will reject.
	// Then the invalid nick callback will be generated.
	// Hacky, but it works.
	/////////////////////////////////////////////////////////////////////////////////////////////
	if(!invalidNick)
		sendingNick = connection->nick;
	else
		sendingNick = "1";

	// Send the nick.
	/////////////////
	ciSocketSendf(&connection->chatSocket, "NICK %s", sendingNick);

	// Send the user.
	/////////////////
	ciSocketSendf(&connection->chatSocket, "USER %s %s %s :%s",
		connection->user,
		"127.0.0.1",
		connection->server,
		connection->name);
}

static CHAT chatConnectDoit(const char * serverAddress,
				 int port,
                 const char * nick,
				 const char * user,
				 const char * name,
				 chatGlobalCallbacks * callbacks,
				 chatNickErrorCallback nickErrorCallback,
				 chatFillInUserCallback fillInUserCallback,
                 chatConnectCallback connectCallback,
                 void * param,
                 CHATBool blocking)
{
	ciConnection * connection;

	assert(serverAddress != NULL);
	ASSERT_NICK();
	assert(callbacks != NULL);
	assert(connectCallback != NULL);

	// Init sockets.
	////////////////
	SocketStartUp();

	// Create a connection object.
	//////////////////////////////
	connection = (ciConnection *)gsimalloc(sizeof(ciConnection));
	if(connection == NULL)
		return NULL;  //ERRCON

	// Initialize the connection.
	/////////////////////////////
	memset(connection, 0, sizeof(ciConnection));
	strcpy(connection->nick, nick);
	if(user)
		strcpy(connection->user, user);
	strcpy(connection->name, name);
	strcpy(connection->server, serverAddress);
	connection->port = port;
	connection->globalCallbacks = *callbacks;
/*	if(gethostname(connection->host, MAX_HOST) == SOCKET_ERROR)
		strcpy(connection->host, "<unknown>"); //ERRCON*/
	connection->nextID = 1;
	connection->connecting = CHATTrue;
	connection->quiet = CHATFalse;

	// Initialize the channel table.
	////////////////////////////////
	if(!ciInitChannels(connection))
	{
		gsifree(connection);
		SocketShutDown();
		return NULL; //ERRCON
	}

	// Initialize the callbacks list.
	/////////////////////////////////
	if(!ciInitCallbacks(connection))
	{
		ciCleanupChannels((CHAT)connection);
		gsifree(connection);
		SocketShutDown();
		return NULL; //ERRCON
	}

	// Initialize the socket.
	/////////////////////////
	if(!ciSocketInit(&connection->chatSocket))
	{
		ciCleanupCallbacks((CHAT)connection);
		ciCleanupChannels((CHAT)connection);
		gsifree(connection);
		SocketShutDown();
		return NULL; //ERRCON
	}

	// Connect the socket.
	//////////////////////
	if(!ciSocketConnect(&connection->chatSocket, serverAddress, port))
	{
		ciSocketDisconnect(&connection->chatSocket);
		ciCleanupCallbacks((CHAT)connection);
		ciCleanupChannels((CHAT)connection);
		gsifree(connection);
		SocketShutDown();
		return NULL; //ERRCON
	}

	// Special stuff for MS Chat server.
	////////////////////////////////////
	//ciSocketSend(&connection->chatSocket, "MODE ISIRCX");
	//ciSocketSend(&connection->chatSocket, "IRCX");

	// Set the callback info.
	/////////////////////////
	connection->nickErrorCallback = nickErrorCallback;
	connection->fillInUserCallback = fillInUserCallback;
	connection->connectCallback = connectCallback;
	connection->connectParam = param;

	if(connection->fillInUserCallback)
	{
		// Get the IP.
		//////////////
		ciSocketSendf(&connection->chatSocket, "USRIP");
	}
	else
	{
		ciSendNickAndUser((CHAT)connection);
	}

	// Do blocking.
	///////////////
	if(blocking)
	{
		// While we're connecting.
		//////////////////////////
		do
		{
			ciThink((CHAT)connection, 0);
            Sleep( 10 );
		} while(connection->connecting);

		// Check if the connect failed.
		///////////////////////////////
		if(!connection->connected)
		{
			// Disconnect the connection.
			/////////////////////////////
			chatDisconnect((CHAT)connection);
			connection = NULL;
		}
	}

	return (CHAT)connection;
}

CHAT chatConnect(const char * serverAddress,
				 int port,
                 const char * nick,
				 const char * user,
				 const char * name,
				 chatGlobalCallbacks * callbacks,
				 chatNickErrorCallback nickErrorCallback,
                 chatConnectCallback connectCallback,
                 void * param,
                 CHATBool blocking)
{
	return chatConnectDoit(serverAddress,
		port,
		nick,
		user,
		name,
		callbacks,
		nickErrorCallback,
		NULL,
		connectCallback,
		param,
		blocking);
}

void chatRetryWithNick(CHAT chat,
					   const char * nick)
{
	CONNECTION;

	// Are we already connected?
	////////////////////////////
	if(connection->connected)
		return;

	// Check for a bad nick.
	////////////////////////
	if(!nick || !nick[0] || (strlen(nick) >= MAX_NICK))
	{
		if(connection->connectCallback != NULL)
			connection->connectCallback(chat, CHATFalse, connection->connectParam);

		return;
	}

	// Send the new nick.
	/////////////////////
	ciSocketSendf(&connection->chatSocket, "NICK :%s", nick);

	// Copy the new nick.
	/////////////////////
	strcpy(connection->nick, nick);
}

void chatDisconnect(CHAT chat)
{
	CONNECTION;

	// Cleanup all the filters first.
	/////////////////////////////////
	ciCleanupFilters(chat);

	// Call the disconnected callback if we haven't already.
	////////////////////////////////////////////////////////
	if(!connection->disconnected && connection->globalCallbacks.disconnected)
		connection->globalCallbacks.disconnected(chat, "", connection->globalCallbacks.param);

	// Are we connected.
	////////////////////
	if(connection->connected)
	{
		ciSocketSend(&connection->chatSocket, "QUIT :Later!");
		ciSocketThink(&connection->chatSocket);
	}

	// gsifree the channel table.
	//////////////////////////
	ciCleanupChannels(chat);

	// Cleanup the callbacks list.
	//////////////////////////////
	ciCleanupCallbacks(chat);

	// Shutdown the chat socket.
	////////////////////////////
	ciSocketDisconnect(&connection->chatSocket);

	// gsifree the memory.
	///////////////////
	gsifree(chat);

	// Shutdown sockets.
	////////////////////
	SocketShutDown();
}

void chatThink(CHAT chat)
{
	ciThink(chat, 0);
}

/*************
** CHANNELS **
*************/
void chatEnterChannel(CHAT chat,
					  const char * channel,
					  const char * password,
					  chatChannelCallbacks * callbacks,
					  chatEnterChannelCallback callback,
					  void * param,
					  CHATBool blocking)
{
	int ID;
	CONNECTION;
	CONNECTED;

	ASSERT_CHANNEL();
	assert(callbacks != NULL);

	if(password == NULL)
		password = "";

	ciSocketSendf(&connection->chatSocket, "JOIN %s %s", channel, password);

	ID = ciAddJOINFilter(chat, channel, callback, param, callbacks, password);

	// Entering.
	////////////
	ciChannelEntering(chat, channel);

	CI_DO_BLOCKING;
}

void chatLeaveChannel(CHAT chat,
					  const char * channel,
					  const char * reason)
{
	CONNECTION;
	CONNECTED;

	ASSERT_CHANNEL();

	if(!reason)
		reason = "";

	ciSocketSendf(&connection->chatSocket, "PART %s :%s", channel, reason);

	// Left the channel.
	////////////////////
	ciChannelLeft(chat, channel);
}

void chatSendChannelMessage(CHAT chat,
							const char * channel,
							const char * message,
							int type)
{
	chatChannelCallbacks * callbacks;
	CONNECTION;
	CONNECTED;

	ASSERT_CHANNEL();
	ASSERT_TYPE(type);

	if (!message || !message[0])
		return;
	if(type == CHAT_MESSAGE)
		ciSocketSendf(&connection->chatSocket, "PRIVMSG %s :%s", channel, message);
	else if(type == CHAT_ACTION)
		ciSocketSendf(&connection->chatSocket, "PRIVMSG %s :\001ACTION %s\001", channel, message);
	else if(type == CHAT_NOTICE)
		ciSocketSendf(&connection->chatSocket, "NOTICE %s :%s", channel, message);
	else if(type == CHAT_UTM)
		ciSocketSendf(&connection->chatSocket, "UTM %s :%s", channel, message);
	else if(type == CHAT_ATM)
		ciSocketSendf(&connection->chatSocket, "ATM %s :%s", channel, message);
	else
		return;

	// We don't get these back, so call the callbacks.
	//////////////////////////////////////////////////
	callbacks = ciGetChannelCallbacks(chat, channel);
	if(callbacks != NULL)
	{
		ciCallbackChannelMessageParams params;
		params.channel = (char *)channel;
		params.user = connection->nick;
		params.message = (char *)message;
		params.type = type;
		ciAddCallback(chat, CALLBACK_CHANNEL_MESSAGE, callbacks->channelMessage, &params, callbacks->param, 0, channel);
	}
}

/**********
** USERS **
**********/
static void ciEnumUsersCallback(CHAT chat, const char * channel, int numUsers, const char ** users, int * modes, void * param)
{
	ciEnumUsersData * data;
	CONNECTION;
    (void) connection;

	// Check the args.
	//////////////////
	ASSERT_CHANNEL();
	assert(numUsers >= 0);
#ifdef _DEBUG
	{
	int i;
	if(numUsers > 0)
	{
		assert(users != NULL);
		assert(modes != NULL);
	}
	for(i = 0 ; i < numUsers ; i++)
	{
		ASSERT_USER(users[i]);
		ASSERT_TYPE(modes[i]);
	}
	}
#endif
	assert(param != NULL);

	// Get the data.
	////////////////
	data = (ciEnumUsersData *)param;
	assert(data->callback != NULL);

	// Call the callback directly.
	//////////////////////////////
	data->callback(chat, CHATTrue, channel, numUsers, users, modes, data->param);
}

void chatEnumUsers(CHAT chat,
				   const char * channel,
				   chatEnumUsersCallback callback,
				   void * param,
				   CHATBool blocking)
{
	int ID;
	ciEnumUsersData data;
	CONNECTION;
	CONNECTED;

	//ASSERT_CHANNEL();
	assert(callback != NULL);

	if(channel == NULL)
		channel = "";

	// Is there a channel specified?
	////////////////////////////////
	if(channel[0] != '\0')
	{
		// Check if we have this one locally.
		/////////////////////////////////////
		if(ciInChannel(chat, channel))
		{
			// Get the users in the channel.
			////////////////////////////////
			data.callback = callback;
			data.param = param;
			ciChannelListUsers(chat, channel, ciEnumUsersCallback, &data);

			return;
		}
	}

	ciSocketSendf(&connection->chatSocket, "NAMES %s", channel);

	// Channel needs to be empty, not NULL, for the filter.
	///////////////////////////////////////////////////////
	if(!channel[0])
		channel = NULL;

	ID = ciAddNAMESFilter(chat, channel, callback, param);
	
	CI_DO_BLOCKING;
}

void chatSendUserMessage(CHAT chat,
						 const char * user,
						 const char * message,
						 int type)
{
	CONNECTION;
	CONNECTED;

	ASSERT_USER(user);
	ASSERT_TYPE(type);

	if (!message || message[0] == 0)
		return;	
	
	if(type == CHAT_MESSAGE)
		ciSocketSendf(&connection->chatSocket, "PRIVMSG %s :%s", user, message);
	else if(type == CHAT_ACTION)
		ciSocketSendf(&connection->chatSocket, "PRIVMSG %s :\001ACTION %s\001", user, message);
	else if(type == CHAT_NOTICE)
		ciSocketSendf(&connection->chatSocket, "NOTICE %s :%s", user, message);
	else if(type == CHAT_UTM)
		ciSocketSendf(&connection->chatSocket, "UTM %s :%s", user, message);
	else if(type == CHAT_ATM)
		ciSocketSendf(&connection->chatSocket, "ATM %s :%s", user, message);
}

void chatGetUserInfo(CHAT chat,
					 const char * user,
					 chatGetUserInfoCallback callback,
					 void * param,
					 CHATBool blocking)
{
	int ID;
	CONNECTION;
	CONNECTED;

	ASSERT_USER(user);
	assert(callback != NULL);

	ciSocketSendf(&connection->chatSocket, "WHOIS %s", user);

	ID = ciAddWHOISFilter(chat, user, callback, param);

	CI_DO_BLOCKING;
}




