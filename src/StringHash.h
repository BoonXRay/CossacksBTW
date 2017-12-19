#ifndef STRINGHASH_H
#define STRINGHASH_H

struct StrHashItem
{
	int NStr;
	int MaxStr;
	char** Str;
	int* Value;
};

class StrHash
{
public:
	StrHashItem SHI[256];
	int LastIndex;
    void AddString( const char * str );
    int SearchString( const char* str);
	void Clear();
	StrHash();
	~StrHash();
};

#endif // STRINGHASH_H
