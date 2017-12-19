#ifndef SHIPTRACE_H
#define SHIPTRACE_H

#include <cstdint>

extern uint8_t * RivDir;

extern bool RiverEditMode;

extern uint8_t * RivVol;

void InitRiv();

void InitBlobs();
void LoadBlobs();
void AddRandomBlobs();
void TestBlob();

void ProcessRivEditor();

class OneObject;
void CreateTrace( OneObject * OB );

#endif // SHIPTRACE_H
