#ifndef cmdParserV3Settings_h__
#define cmdParserV3Settings_h__

////==show debug info for serial parser
////(comment to turn output off)

//#define SERIAL_PARSER_SHOW_DEBUG
//#define SERIAL_PARSER_SHOW_MESSAGES

//=== if uncommented - command must be started by SERIAL_COMMAND_PREFIX_CHAR
//===all chars after the end of the last command and until a new prefix char is received will be lost
#define USE_SERIAL_COMMAND_PREFIX
#define SERIAL_COMMAND_PREFIX_CHAR '~'
//#define SERIAL_COMMAND_PREFIX_CHAR '#'


//---!!!cannot be used yet!!! (underconstr., may be not necessary at all)
//#define USE_SERIAL_COMMAND_TERMINATOR
//#define SERIAL_COMMAND_TERMINATOR_CHAR ';'


#define SERCMDPARSER__SERIAL_BUFFER_SIZE 60
#define SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE 20
#define SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS 8
//---set that value to 0 to turn that option off
#define SERCMDPARSER__INTERCHAR_TIMEOUT_MILLIS 300
#define SERCMDPARSER__COMMAND_TIMEOUT_MILLIS 800

//====service lines, keep it here

//===this line is necessary to check if that file exists into scetch folder 
//===(because Arduino uses outdated C++ pre-processor version and cannot check if file exists)
#define CMD_PARSER_LOCALSETTINGS_OK
#define CMD_PARSER_VERSION 3
#endif // cmdParserV3Settings_h__