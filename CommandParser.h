//TO DO:
//make customizable output, variants:
//0: no any output, just parse and execute
//1: show brief error messages
//2: same as 1+show a description of command (a most common case)
//3: same as 2+show detailed info on error
//4: same as 3+show detailed info in any case

//--- add example for waitForSingleChar();

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

//----DESCRIPTION--------------------------------------\
//=create an array of possible commands (with callback functions)

//=receive serial input
//-when e-o-l or maximum lenght of buffer reached - stop receiving 
//-when command char recognized - parse parameters
//-call a 'callback' function from array
//-called function can obtain parameters using "getParameterXX" functions;
//-----------------------------------------------------/

////==show debug info for serial parser
////(comment to turn output off)
//#define DEBUG_SERIAL_PARSER 1


#define SERCMDPARSER__SERIAL_BUFFER_SIZE 30
#define SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE 10
#define SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS 3

//just predefined (and very unprobable in normal condition) value
#define SERCMDPARSER__INTEGER_NULLVALUE 0xFFF0


#include <Arduino.h>
//======================================
//
//======================================
struct SerialCommand{
	char cmdChar;
	const __FlashStringHelper* description;
	void (*function)();
	uint8_t numberOfParameters;
	};
//======================================
//
//======================================	

class SerialCommandParser
{
public:
	SerialCommandParser();
	void addCommand(char parChar, const __FlashStringHelper * parDescription, void (*function)(), uint8_t parNumberOfParameters);
	void PrintCommandList();
	//call this function as frequently as it possible
	void parseSerialInput();
		
	void clearSerialCommand();
	void syntaxHalt( const __FlashStringHelper * parErrorMessage);
	void processParserError( const __FlashStringHelper * parErrorMessage);
	void showDebugInfo();
	int getParameterInt(uint8_t paramParamNo);
	char *getParameterString(uint8_t paramParamNo);
	char getParameterChar(uint8_t paramParamNo);
	int stringToInt(char * parString);
	char waitForSingleChar( const __FlashStringHelper * parMessage);
private:
	
	SerialCommand commandsArray[SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE];
	
	char serialBuffer[SERCMDPARSER__SERIAL_BUFFER_SIZE];//buffer to receive from Serial port
	size_t numberOfCommands=0; //total number of predefined commands 
	size_t inputCursorPosition;//position of next symbol will be received from serial buffer
	SerialCommand *currentParsedCommand;
	bool commandStringReceived=true; //flag, when command is received - stop reading of input and parse serial
	uint8_t numberOfParameters; //number of parameters parsed in current command
	//pointers to recognized (space-separated) parameters at inputline
	char *parameters[SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS];
	
	void checkParameterNoOrHalt(int paramParamNo);
	
};

SerialCommandParser extern commandParser;
#endif //\COMMAND_PARSER_H
