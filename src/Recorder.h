#ifndef RECORDER_H
#define RECORDER_H

#include <cstdint>

class InternetStream{
public:
	bool LocalFile;
	int CurPosition;
	char StreamName[128];
	char Server[8];
	char HashName[128];
	bool Open(char* Name);
	void Close();
    int ReadSomething( uint8_t * Buf,int MaxLen,bool Scroll,bool Blocking);
	int Error();
};
class RecordGame{
public:
	InternetStream STREAM;
    uint8_t * Stream;
	int Size;
	int MaxSize;
	int Pos;
	int LastSentPos;
	char MapName[64];
	char RecName[64];
	void StartRecord(char* MapName);
	void CloseRecord();
	void AddRecord();
    void AddByte(uint8_t v);
	void AddShort(short v);
    void AddBuf( uint8_t * Data,int Size);
	void ReadStream(char* Name);
	void TryToFlushNetworkStream(bool Final);
	bool Extract();
	void Save();
	void RunRecordedGame(char* Name);
	RecordGame();
	~RecordGame();
};

extern RecordGame RGAME;
extern bool RecordMode;
extern int CurrentAnswer;
extern uint8_t CHOPT;
extern char LASTCHATSTR[512];
extern unsigned int RunMethod;
extern int RunDataSize;
extern uint8_t RunData[2048];

extern int LastScrollTime;

void WritePitchTicks();
void ReadPichTicks();

__declspec( dllexport ) void PlayRecfile( char* RecFile );

#endif // RECORDER_H
