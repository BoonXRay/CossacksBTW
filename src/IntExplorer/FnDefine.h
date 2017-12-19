#ifndef FNDEFINE_H
#define FNDEFINE_H


class sicExplorer;

extern sicExplorer* REQSXP;
extern char* GREQUEST;

class OneInterfaceFunction;
extern OneInterfaceFunction* IFNS;

extern int N_IFNS;

void SendSmartRequest( sicExplorer* SXP, char* Str );

void InitIFNS();

#endif // FNDEFINE_H
