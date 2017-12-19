/*             Work with the resource files
 *   
 *  You must use this module for  accesss to files.This 
 * routine allows you to read  files from disk  or from 
 * the resource file, you even will not recognise where
 * the given file is.
 */

#ifndef RESFILE_H
#define RESFILE_H

#include <windows.h>

#include "Arc/GSCSet.h"

extern CGSCset GSFILES;

//#include "Arc\GSCSet.h"
//typedef LPGSCfile ResFile;
typedef HANDLE ResFile;
//Opening the resource file
__declspec( dllexport ) ResFile RReset(LPCSTR lpFileName);
//Rewriting file
__declspec( dllexport ) ResFile RRewrite(LPCSTR lpFileName);
//Getting size of the resource file
__declspec( dllexport ) DWORD RFileSize(ResFile hFile);
// Setting file position 
__declspec( dllexport ) DWORD RSeek(ResFile hFile,int pos);
//Reading the file
__declspec( dllexport ) DWORD RBlockRead(ResFile hFile,LPVOID lpBuffer,DWORD BytesToRead);
//Writing the file
// BoonXRay 11.09.2017
//DWORD RBlockWrite(ResFile hFile,LPVOID lpBuffer,DWORD BytesToWrite);
__declspec( dllexport ) DWORD RBlockWrite(ResFile hFile,LPCVOID lpBuffer,DWORD BytesToWrite);
//Returns last error
__declspec( dllexport ) DWORD IOresult(void);
//Close the file
__declspec( dllexport ) void RClose(ResFile hFile);

extern bool ProtectionMode;

bool FilesInit();
void FilesExit();

#endif // RESFILE_H
