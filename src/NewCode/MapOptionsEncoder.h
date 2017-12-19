#ifndef MAPOPTIONSENCODER_H
#define MAPOPTIONSENCODER_H

//Encodes 10 given option values into a 7-digit number for map name
//Reason: fixed network buffer size with limited mapname space
int EncodeOptionsInNumber( int *selected_opt_values );

//Decodes 7-digit number into game settings
//Saves settings values in result[10]
//Returns 'Start options' value
int DecodeOptionsFromNumber( const int number, int *result );

#endif // MAPOPTIONSENCODER_H
