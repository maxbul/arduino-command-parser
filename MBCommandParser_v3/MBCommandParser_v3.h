//TO DO:
//====BUG
//when I use prefix and send 2 commands without delimiter ($h$h) - the whole line should be ignored

//move parser prefix to setup() function parameter

//===check where "clearSerialCommand" is used. It may clear a built-in buffer if external bufer used
//---re-make a reading of the serial stream and re-make a parser splitting using strok
///!!!!! MAKE timeout CODE SWITCHABLE by preprocessor (check #defines for timeouts)

//=== check duplicate chars in addCommand()
//!!!!----ERROR!!!!
//[command:]'c' :	clear all(0 params.)
//[sent to serial:] c s A23456789012345
//(2 extra params)
//[OUTPUT:]
//SERL.CMDs.PARSER ERROR:Too many parameters
//Command skipped!
//[RESULT:]command "clear" was really executed!!!!
//!!! check it!!!
//-------------------------
//--make debug printing of the  whole buffer after every incoming char switchable (that's too slow)
//===make output customizable (by DEFINES), variants:
//0: no any output, just parse and execute
//01: make a function to read and parse a list messages
// (make a structure:slot, state, sender (?))
//--make messages shorter

//1: make switchable ALL error messages (print error codes instead of strings)
//1-4 done
//5: compare HWSerial.parseInt with stringToInt, do something between 2 of that (skip initial spaces, take in account initial '-')
//6: add parseFloat (similar to 5)
//7: add SoftwareSerial 
//8: Add usage of queue for strings (check compatibility with StringQueueArrayMB )
//10: ? add default handler function for unknown commands  (to process ANY string)
//--11 DONE
//13 think about "comma-separated" parameters (not only spaces will be replaced by '\0' chars, but ','  or ';')
//may be better to make it switchable?
//--- add example for waitForSingleChar() and waitForAKey();

#ifndef COMMAND_PARSER_V3_H
#define COMMAND_PARSER_V3_H

#define SERCMDPARSER_WHOLE_LINE_AS_FIRST_PARAMETER 255
//----DESCRIPTION--------------------------------------
//=create an array of possible commands (with callback functions)

//=receive serial input
//-when e-o-l or maximum lenght of buffer reached - stop receiving 
//-when command char recognized - parse parameters
//-call a 'callback' function from array
//-called function can obtain parameters using "getParameterXX" functions;
//-----------------------------------------------------

#include <Arduino.h>

//
//!!! if you have compilation error here - that means a local sketch folder doesn't contains
//!!! a file  "cmdParserV3Settings.h"
//!!!(AVR uses gcc-4.9 version and pre-processor cannot check if file exists)
//!!!
//!!! Use cmdParserV3Settings_EXAMPLE.h as a template when create such file
#include "cmdParserV3Settings.h"
//!!!

#ifndef SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE
	#error Check file "cmdParserV3Settings.h"
#endif
#ifndef SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS
	#error Check file "cmdParserV3Settings.h"
#endif
#ifndef SERCMDPARSER__INTERCHAR_TIMEOUT_MILLIS 
	#error Check file "cmdParserV3Settings.h"
#endif
#ifndef SERCMDPARSER__COMMAND_TIMEOUT_MILLIS 
	#error Check file "cmdParserV3Settings.h"
#endif

#ifndef ARRAYSIZE
	#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))
#endif

#define SERCMDPARSER__INTEGER_NULLVALUE	0xFFFD

#ifdef USE_SERIAL_COMMAND_PREFIX
	#ifndef SERIAL_COMMAND_PREFIX_CHAR
	#error to use serial command preffix you must define SERIAL_COMMAND_PREFIX_CHAR
	#endif
#endif

//---not used yet
//#ifdef USE_SERIAL_COMMAND_TERMINATOR
//#ifndef SERIAL_COMMAND_TERMINATOR_CHAR
//#error to use serial command terminator you must define SERIAL_COMMAND_TERMINATOR_CHAR
//#endif
//#endif


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

class MBCommandParser_v3
{
public:
	//SerialCommandParser_v3(Stream *parSerial);
	MBCommandParser_v3(Stream *parSerial, Stream *parDebugSerial);
	void addCommand(char parChar, const __FlashStringHelper * parDescription, void (*function)(), uint8_t parNumberOfParameters);
	
	//use it when you need to get a copy of unchanged serial input
	void storeCopyOfIncomingBuffer(char * parCopy);
	
	//---debug functions
	void printCommandList();
	void printOptionsSettings();
	void showDebugInfo();
	void showGeneralInfo();
	
	//call this function as frequently as it possible
	void processSerialReading();
	boolean parseSerialCommand();
	boolean parseBuffer(char * parBuffer, size_t parSize);
	void loop();
	void clearSerialInputBuffer();		
	void clearSerialCommand();
	void syntaxHalt( const __FlashStringHelper * parErrorMessage);
	void processParserError( const __FlashStringHelper * parErrorMessage);
	
	long getParameterInt(uint8_t paramParamNo);
	float getParameterFloat(uint8_t paramParamNo);
	char *getParameterString(uint8_t paramParamNo);
	size_t copyStringParameterToBuffer(uint8_t paramParamNo, char* parBuff, size_t parSizeOfBuffer);
	
	char getParameterChar(uint8_t paramParamNo);
	long stringToInt(char  parString[]);
	float stringToFloat(const char parString[]);
	char waitForSingleChar( const __FlashStringHelper parMessage[]);
	bool waitForAKey( const __FlashStringHelper parMessage[], char parDesiredKey);
	void setMaxLockingTime(unsigned long parMaxLockingTime);
	void setIntercharTimeout(unsigned long  parTimeoutMillis);
	//process serial reading
	uint8_t readStringToBuffer(char parString[], uint8_t parMaxLenght, char parendingChar, uint32_t parTimeout);

	char * getBuffer();
	
	private:
	Stream	*_serial;
	
	Stream	*_debugSerial;
	
	unsigned _maxLockingTime=0;

	SerialCommand commandsArray[SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE];
	
	
	char _serialBuffer[SERCMDPARSER__SERIAL_BUFFER_SIZE];//buffer to receive from Serial port
public://!!!!
	size_t _numberOfCommands=0; //total number of predefined commands 
private: //!!!	
	size_t _inputCursorPosition;//position of next symbol will be received from serial buffer
	SerialCommand *_currentParsedCommand;
	bool _commandStringReceived=true; //flag, when command is received - stop reading of input and parse serial
	
	uint8_t _numberOfParameters; //number of parameters parsed in current command
	//pointers to recognized (space-separated) parameters at inputline
	char *parameters[SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS];
	
	void checkParameterNoOrHalt(int paramParamNo);
	//bool validateParameter(uint8_t parNumber, uint32_t parMinValue, )
	unsigned long _maxInterCharTimeout=0;
	unsigned long _maxCommandTimeout=0;
	unsigned long _lastIncomingCharMillis;
	unsigned long _lastCommandStartedMillis;
	
	//char *_copyOfIncomingBuffer=NULL;

	
};

//SerialCommandParser extern commandParser;
#endif //\COMMAND_PARSER_V3_H