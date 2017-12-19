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

#ifndef _CHATHANDLERS_H_
#define _CHATHANDLERS_H_

/*************
** INCLUDES **
*************/
#include "chat.h"
#include "chatSocket.h"
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__) && !defined(__mips64)
	#include "::nonport.h"
#else
    #include "nonport.h"
#endif

/**********
** TYPES **
**********/
typedef struct ciServerMessageType
{
	char * command;
	void (* handler)(CHAT chat, ciServerMessage * message);
} ciServerMessageType;

typedef char ** ciCommands;
typedef struct ciServerMessageFilter
{
	int type;
	unsigned long timeout;

	char * name;
	char * name2;

	void * callback;
	void * callback2;
	void * param;

	void * data;

	int ID;

	struct ciServerMessageFilter * pnext;
} ciServerMessageFilter;

/************
** GLOBALS **
************/
extern ciServerMessageType serverMessageTypes[];
extern int numServerMessageTypes;

/**************
** FUNCTIONS **
**************/
void ciFilterThink(CHAT chat);
void ciCleanupFilters(CHAT chat);
int ciAddJOINFilter(CHAT chat, const char * channel, chatEnterChannelCallback callback, void * param, chatChannelCallbacks * callbacks, const char * password);
int ciAddNAMESFilter(CHAT chat, const char * channel, chatEnumUsersCallback callback, void * param);
int ciAddWHOISFilter(CHAT chat, const char * user, chatGetUserInfoCallback callback, void * param);
int ciGetNextID(CHAT chat);
CHATBool ciCheckFiltersForID(CHAT chat, int ID);

#endif
