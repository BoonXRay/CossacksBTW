#ifndef UNSYNCRO_H
#define UNSYNCRO_H

#define MaxUns 100
#define maxUline 5

typedef char* lpCHAR;
char* its(int i);

struct RandSlot
{
    const char* FileName;
	int Line;
	int Param;
	int rpos;
	int Type;
};

class Syncro
{
public:
	RandSlot RSL[maxUline];
	int NSlots;

	FILE* Outf;
	void Clear();
	void Save();
	void Copy(Syncro* Syn);
    Syncro() {}
    ~Syncro() {}
};

extern Syncro SYN;
extern Syncro SYN1;

int RandNew( const char* File, int Line );

#define rando() RandNew(__FILE__,__LINE__)

#endif // UNSYNCRO_H
