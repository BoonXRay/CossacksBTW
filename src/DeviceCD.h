#ifndef DEVICECD_H
#define DEVICECD_H

#include <cstdint>

#include <mmsystem.h>
#include <windows.h>

class CDeviceCD
{
// Construction
public:
	CDeviceCD();

// Attributes
public:

// Operations
public:

// Implementation
public:
	bool Play(DWORD Track);
	bool SetVolume(DWORD Volume);
    uint32_t GetVolume();
	bool Stop();
	bool Resume();
	bool Pause();
	bool Close();
	bool Open();
	virtual ~CDeviceCD();

	// Generated message map functions
protected:
	bool FOpened;
	MCIDEVICEID FDeviceID;
	MCIERROR FError;
	LRESULT MCINotify(WPARAM wFlags, LONG lDevId);
};

void PlayCDTrack(int Id);
void PlayRandomTrack();

LRESULT CD_MCINotify(WPARAM wFlags, LONG lDevId);

void StopPlayCD();
int GetCDVolume();
void SetCDVolume( int );

#endif // DEVICECD_H
