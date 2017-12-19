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

#ifndef _CHAT_H_
#define _CHAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/************
** DEFINES **
************/
// User and channel message types.
//////////////////////////////////
#define CHAT_MESSAGE        0
#define CHAT_ACTION         1
#define CHAT_NOTICE         2
#define CHAT_UTM            3
#define CHAT_ATM            4

// User modes.
// PANTS|03.12.01 - These are now bitflags!
// Both CHAT_VOICE and CHAT_OP can be set at the same time.
///////////////////////////////////////////////////////////
#define CHAT_NORMAL         0
#define CHAT_VOICE          1
#define CHAT_OP             2

// Part reasons (see the chatUserParted callback).
//////////////////////////////////////////////////
#define CHAT_LEFT           0  // The user left the channel.
#define CHAT_QUIT           1  // The user quit the chat network.
#define CHAT_KICKED         2  // The user was kicked from the channel.
#define CHAT_KILLED         3  // The user was kicked off the chat network.

// Possible nick errors while connecting.
/////////////////////////////////////////
#define CHAT_IN_USE         0
#define CHAT_INVALID        1

/**********
** TYPES **
**********/
// Boolean type.
////////////////
typedef enum { CHATFalse, CHATTrue } CHATBool;

// A CHAT object represents a client connection to a chat server.
/////////////////////////////////////////////////////////////////
typedef void * CHAT;

// Object representing a channel's mode.
////////////////////////////////////////
typedef struct CHATChannelMode
{
	CHATBool InviteOnly;
	CHATBool Private;
	CHATBool Secret;
	CHATBool Moderated;
	CHATBool NoExternalMessages;
	CHATBool OnlyOpsChangeTopic;
	int Limit;
} CHATChannelMode;

// The result of a channel enter attempt,
// passed into the chatEnterChannelCallback().
//////////////////////////////////////////////
typedef enum
{
	CHATEnterSuccess,        // The channel was successfully entered.
	CHATBadChannelName,      // The channel name was invalid.
	CHATChannelIsFull,       // The channel is at its user limit.
	CHATInviteOnlyChannel,   // The channel is invite only.
	CHATBannedFromChannel,   // The local user is banned from this channel.
	CHATBadChannelPassword,  // The channel has a password, and a bad password (or none) was given.
	CHATTooManyChannels,     // The server won't allow this user in any more channels.
	CHATEnterTimedOut,       // The attempt to enter timed out.
	CHATBadChannelMask       // Not sure if any servers use this, or what it means! (ERR_BADCHANMASK)
} CHATEnterResult;

/**********************
** GLOBALS CALLBACKS **
**********************/
// Gets raw incoming network traffic.
/////////////////////////////////////
typedef void (* chatRaw)(CHAT chat,
						 const char * raw,
						 void * param);

// Called when the client has been disconnected.
////////////////////////////////////////////////
typedef void (* chatDisconnected)(CHAT chat,
								  const char * reason,
								  void * param);

// Called when a private message from another user is received.
// If user==NULL, this is a message from the server.
///////////////////////////////////////////////////////////////
typedef void (* chatPrivateMessage)(CHAT chat,
									const char * user,
									const char * message,
									int type,  // See defined message types above.
									void * param);

// Called when invited into a channel.
//////////////////////////////////////
typedef void (* chatInvited)(CHAT chat,
							 const char * channel,
							 const char * user,
							 void * param);

// A connection's global callbacks.
///////////////////////////////////
typedef struct chatGlobalCallbacks
{
	chatRaw raw;
	chatDisconnected disconnected;
	chatPrivateMessage privateMessage;
	chatInvited invited;
	void * param;
} chatGlobalCallbacks;

/**********************
** CHANNEL CALLBACKS **
**********************/
// Called when a message is received in a channel.
//////////////////////////////////////////////////
typedef void (* chatChannelMessage)(CHAT chat,
									const char * channel,
									const char * user,
									const char * message,
									int type,  // See defined message types above.
									void * param);

// Called when the local client is kicked from a channel.
/////////////////////////////////////////////////////////
typedef void (* chatKicked)(CHAT chat,
							const char * channel,
							const char * user,
							const char * reason,
							void * param);

// Called when a user joins a channel we're in.
///////////////////////////////////////////////
typedef void (* chatUserJoined)(CHAT chat,
								const char * channel,
								const char * user,
								int mode,    // See defined user modes above.
								void * param);

// Called when a user parts a channel we're in.
///////////////////////////////////////////////
typedef void (* chatUserParted)(CHAT chat,
								const char * channel,
								const char * user,
								int why,    // See defined part reasons above.
								const char * reason,
								const char * kicker,
								void * param);

// Called when a user in a channel we're in changes nicks.
//////////////////////////////////////////////////////////
typedef void (* chatUserChangedNick)(CHAT chat,
									 const char * channel,
									 const char * oldNick,
									 const char * newNick,
									 void * param);

// Called when the topic changes in a channel we're in.
///////////////////////////////////////////////////////
typedef void (* chatTopicChanged)(CHAT chat,
								  const char * channel,
								  const char * topic,
								  void * param);

// Called when the mode changes in a channel we're in.
//////////////////////////////////////////////////////
typedef void (* chatChannelModeChanged)(CHAT chat,
										const char * channel,
										CHATChannelMode * mode,
										void * param);

// Called when a user's mode changes in a channel we're in.
///////////////////////////////////////////////////////////
typedef void (* chatUserModeChanged)(CHAT chat,
									 const char * channel,
									 const char * user,
									 int mode,  // See defined user modes above.
									 void * param);

// Called when the user list changes (due to a join or a part) in a channel we're in.
/////////////////////////////////////////////////////////////////////////////////////
typedef void (* chatUserListUpdated)(CHAT chat,
									 const char * channel,
									 void * param);

// Called when the chat server sends an entire new user list for a channel we're in.
////////////////////////////////////////////////////////////////////////////////////
typedef void (* chatNewUserList)(CHAT chat,
								 const char * channel,
								 int num,
								 const char ** users,
								 int * modes,
								 void * param);

// Called when a user changes a broadcast key in a channel we're in.
////////////////////////////////////////////////////////////////////
typedef void (* chatBroadcastKeyChanged)(CHAT chat,
										 const char * channel,
										 const char * user,
										 const char * key,
										 const char * value,
										 void * param);

// A channel's callbacks.
/////////////////////////
typedef struct chatChannelCallbacks
{
	chatChannelMessage channelMessage;
	chatKicked kicked;
	chatUserJoined userJoined;
	chatUserParted userParted;
	chatUserChangedNick userChangedNick;
	chatTopicChanged topicChanged;
	chatChannelModeChanged channelModeChanged;
	chatUserModeChanged userModeChanged;
	chatUserListUpdated userListUpdated;
	chatNewUserList newUserList;
	chatBroadcastKeyChanged broadcastKeyChanged;
	void * param;
} chatChannelCallbacks;

/************
** GENERAL **
************/
// Called when a connect attempt completes.
///////////////////////////////////////////
typedef void (* chatConnectCallback)(CHAT chat,
									 CHATBool success,
									 void * param);
// Called if there is an error with the nick while connecting.
// To retry with a new nick, call chatRetryWithNick.
// Otherwise, call chatDisconnect to stop the connection.
//////////////////////////////////////////////////////////////
typedef void (* chatNickErrorCallback)(CHAT chat,
									   int type,  // CHAT_IN_USE, CHAT_INVALID
									   const char * nick,
									   void * param);
typedef void (* chatFillInUserCallback)(CHAT chat,
										unsigned int IP, // PANTS|08.21.00 - changed from unsigned long
										char user[128],
										void * param);
// Connects you to a chat server and returns a CHAT object.
///////////////////////////////////////////////////////////
CHAT chatConnect(const char * serverAddress,
				 int port,
                 const char * nick,
				 const char * user,
				 const char * name,
				 chatGlobalCallbacks * callbacks,
				 chatNickErrorCallback nickErrorCallback,
                 chatConnectCallback connectCallback,
                 void * param,
                 CHATBool blocking);
// If the chatNickErrorCallback gets called, then this can be called
// with a new nick to retry.  If this isn't called, the connection can be
// disconnected with chatDisconnect.  If the new nick is successful, then
// the chatConnectCallback will get called.  If there's another nick 
// error, the chatNickErrorCallback will get called again.
/////////////////////////////////////////////////////////////////////////
void chatRetryWithNick(CHAT chat,
					   const char * nick);

// Disconnect the chat connection.
//////////////////////////////////
void chatDisconnect(CHAT chat);

// Processes the chat connection.
/////////////////////////////////
void chatThink(CHAT chat);

// Called as a result of a nick change attempt.
///////////////////////////////////////////////
typedef void (* chatChangeNickCallback)(CHAT chat,
										CHATBool success,
										const char * oldNick,
										const char * newNick,
										void * param);

/*************
** CHANNELS **
*************/
// Gets called for each channel enumerated.
///////////////////////////////////////////
typedef void (* chatEnumChannelsCallbackEach)(CHAT chat,
											  CHATBool success,
											  int index,
											  const char * channel,
											  const char * topic,
											  int numUsers,
											  void * param);
// Gets called after all channels have been enumerated.
///////////////////////////////////////////////////////
typedef void (* chatEnumChannelsCallbackAll)(CHAT chat,
											 CHATBool success,
											 int numChannels,
											 const char ** channels,
											 const char ** topics,
											 int * numUsers,
											 void * param);

// Gets called for each channel enumerated.
///////////////////////////////////////////
typedef void (* chatEnumJoinedChannelsCallback)(CHAT chat,
											  int index,
											  const char * channel,
											  void * param);


// Gets called when a channel has been entered.
///////////////////////////////////////////////
typedef void (* chatEnterChannelCallback)(CHAT chat,
										  CHATBool success,
										  CHATEnterResult result,
										  const char * channel,
										  void * param);
// Enters a channel.
////////////////////
void chatEnterChannel(CHAT chat,
					  const char * channel,
					  const char * password,
					  chatChannelCallbacks * callbacks,
					  chatEnterChannelCallback callback,
					  void * param,
					  CHATBool blocking);

// Leaves a channel.
////////////////////
void chatLeaveChannel(CHAT chat,
					  const char * channel,
					  const char * reason);  // PANTS|03.13.01

// Sends a message to a channel.
////////////////////////////////
void chatSendChannelMessage(CHAT chat,
							const char * channel,
							const char * message,
							int type);

// Gets called when a channel's topic has been retrieved.
/////////////////////////////////////////////////////////
typedef void (* chatGetChannelTopicCallback)(CHAT chat,
											 CHATBool success,
											 const char * channel,
											 const char * topic,
											 void * param);

// Gets called when a channel's mode has been retrieved.
////////////////////////////////////////////////////////
typedef void (* chatGetChannelModeCallback)(CHAT chat,
											CHATBool success,
											const char * channel,
											CHATChannelMode * mode,
											void * param);

// Called when the channel's password has been retrieved.
/////////////////////////////////////////////////////////
typedef void (* chatGetChannelPasswordCallback)(CHAT chat,
												CHATBool success,
												const char * channel,
												CHATBool enabled,
												const char * password,
												void * param);

// Called with the list of bans in a channel.
/////////////////////////////////////////////
typedef void (* chatEnumChannelBansCallback)(CHAT chat,
											 CHATBool success,
											 const char * channel,
											 int numBans,
											 const char ** bans,
											 void * param);

/**********
** USERS **
**********/
// Called with the list of users in a channel.
//////////////////////////////////////////////
typedef void (* chatEnumUsersCallback)(CHAT chat,
									   CHATBool success,
									   const char * channel, //PANTS|02.11.00|added paramater
									   int numUsers,
									   const char ** users,
									   int * modes,
									   void * param);
// Enumerate through the users in a channel.
////////////////////////////////////////////
void chatEnumUsers(CHAT chat,
				   const char * channel,
				   chatEnumUsersCallback callback,
				   void * param,
				   CHATBool blocking);

// Send a private message to a user.
////////////////////////////////////
void chatSendUserMessage(CHAT chat,
						 const char * user,
						 const char * message,
						 int type);

// Called with a user's info.
/////////////////////////////
typedef void (* chatGetUserInfoCallback)(CHAT chat,
										 CHATBool success,
										 const char * nick,  //PANTS|02.14.2000|added nick and user
										 const char * user,
										 const char * name,
										 const char * address,
										 int numChannels,
										 const char ** channels,
										 void * param);
// Get a user's info.
/////////////////////
void chatGetUserInfo(CHAT chat,
					 const char * user,
					 chatGetUserInfoCallback callback,
					 void * param,
					 CHATBool blocking);

// Called with a user's basic info.
///////////////////////////////////
typedef void (* chatGetBasicUserInfoCallback)(CHAT chat,
											  CHATBool success,
											  const char * nick,
											  const char * user,
											  const char * address,
											  void * param);

// Called with a user's basic info for everyone in a channel.
// Called with a NULL nick/user/address at the end.
/////////////////////////////////////////////////////////////
typedef void (* chatGetChannelBasicUserInfoCallback)(CHAT chat,
													 CHATBool success,
													 const char * channel,
													 const char * nick,
													 const char * user,
													 const char * address,
													 void * param);

// Called with the user's mode.
///////////////////////////////
typedef void (* chatGetUserModeCallback)(CHAT chat,
										 CHATBool success,
										 const char * channel,
										 const char * user,
										 int mode,
										 void * param);
/*********
** KEYS **
*********/

// Called with a user's global key/values.
// If used for a set of users, will be
// called with user==NULL when done.
//////////////////////////////////////////

// BoonXRay 03.09.2017
/*typedef void (* chatGetGlobalKeysCallback)(CHAT chat,
										   CHATBool success,
										   const char * user,
										   int num,
                                           const char ** keys,
										   const char ** values,
                                           void * param);*/
typedef void (* chatGetGlobalKeysCallback)(CHAT chat,
                                           CHATBool success,
                                           const char * user,
                                           int num,
                                           char ** keys,
                                           char ** values,
                                           void * param);

// Called with a user's channel key/values, or a channel's key/values.
// If used for a set of users, will be called with user==NULL when done.
// If used for a channel, will be called once with user==NULL.
////////////////////////////////////////////////////////////////////////

// BoonXRay 03.09.2017
/*typedef void (* chatGetChannelKeysCallback)(CHAT chat,
											CHATBool success,
											const char * channel,
											const char * user,
											int num,
											const char ** keys,
											const char ** values,
                                            void * param);*/
typedef void (* chatGetChannelKeysCallback)(CHAT chat,
                                            CHATBool success,
                                            const char * channel,
                                            const char * user,
                                            int num,
                                            char ** keys,
                                            char ** values,
                                            void * param);

#ifdef __cplusplus
}
#endif

#endif
