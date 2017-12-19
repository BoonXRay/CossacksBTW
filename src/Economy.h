#ifndef ECONOMY_H
#define ECONOMY_H

#include <cstdint>

void ReloadECO();

class IconSet;
void EconomyInterface( IconSet* ISET );
void EconomePostDraw();
void LoadEconomy();
void ProcessEconomy();


int GetFinPower( int* Fin, int Nation );

//Calculates exchange rate for the game market information popup
int GetCurrentCost( uint8_t Buy, uint8_t Sell, int Amount );
//Calculates exchange rate for the game market information popup
int GetNominalCost( uint8_t Buy, uint8_t Sell, int Amount );
int GetEconomyData( uint8_t ** EC );

void PerformTorg( uint8_t Nation, uint8_t SellRes, uint8_t BuyRes, int SellAmount );
int GetTorgResultEx( uint8_t SellRes, uint8_t BuyRes, int SellAmount );

#endif // ECONOMY_H
