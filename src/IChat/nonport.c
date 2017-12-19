/******
nonport.c
GameSpy Developer SDK 
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******/

#ifdef __cplusplus
extern "C" {
#endif

#include "nonport.h"

void SocketStartUp()
{
#if defined(_WIN32) || defined(_MACOS)
	WSADATA data;
	WSAStartup(0x0101, &data);
#endif


}

void SocketShutDown()
{
#if defined(_WIN32) || defined(_MACOS)
	WSACleanup();
#endif

}

#include <string.h>
#include <stdlib.h>
char * goastrdup(const char *src)
{
    char *res;
    if(src == NULL)      //PANTS|02.11.00|check for NULL before strlen
        return NULL;
    res = (char *)gsimalloc(strlen(src) + 1);
    if(res != NULL)      //PANTS|02.02.00|check for NULL before strcpy
        strcpy(res, src);
    return res;
}

#ifdef __cplusplus
}
#endif
