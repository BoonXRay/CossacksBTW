/******
nonport.h
GameSpy Developer SDK 
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******/

#ifndef _NONPORT_H_
#define _NONPORT_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define GOAGetLastError(s) WSAGetLastError()
	#include <windows.h>
	#include <winsock.h>
#else
#endif 

#ifdef __cplusplus
extern "C" {
#endif

#define gsimalloc malloc
#define gsifree free
#define gsirealloc realloc
	
void SocketStartUp();
void SocketShutDown();

#if defined(_WIN32) && !defined(UNDER_CE)
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#else
	#include <string.h>
	#include <stdlib.h>
	#include <ctype.h>
	
	char *_strlwr(char *string);
	char *_strupr(char *string);
#endif

#undef strdup
#define strdup goastrdup
#undef _strdup
#define _strdup goastrdup
char * goastrdup(const char *src);

#ifdef __cplusplus
}
#endif


#endif 
