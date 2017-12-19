#include <cstdint>

class GFILE;
class Nation;
bool UnderstandUpgrade(GFILE* f,char* UpgName,Nation* NT,char* name,int* lpLine, uint8_t NID);

class OneObject;
void PerformNewUpgrade(Nation* NT,int UIndex, OneObject* OB);

class AdvCharacter;
class NewMonster;
void CreateAdvCharacter(AdvCharacter* AC,NewMonster* NM);
int GetUpgradeID(Nation* NT,char* Name);
