#ifndef HINT_H
#define HINT_H

#include <cstdint>

extern int HintX;
extern int HintY;

void SetupHint();
void ClearHints();
void AssignHint(const char *s, int time );
void AssignHintLo(char* s, int time);
void CreateTimedHint(const char *s, int time );
void CreateTimedHintEx(const char *s, int time, uint8_t opt );
void ProcessHints();

class GeneralObject;
void GetChar( GeneralObject* GO, char* s );

#endif // HINT_H
