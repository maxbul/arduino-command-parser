#ifndef cmdParserV2Settings_h__
#define cmdParserV2Settings_h__

////==show debug info for serial parser
////(comment to turn output off)
#define SERIAL_PARSER_SHOW_DEBUG
#define SERIAL_PARSER_SHOW_MESSAGES

//=== if uncommented - command must be started by SERIAL_COMMAND_PREFIX_CHAR
//===all chars after the end of the last command and until a new preffix char is received will be lost
#define USE_SERIAL_COMMAND_PREFIX
#define SERIAL_COMMAND_PREFIX_CHAR '$'


//---!!!cannot be used yet!!! (underconstr., may be not necessary at all)
//#define USE_SERIAL_COMMAND_TERMINATOR
//#define SERIAL_COMMAND_TERMINATOR_CHAR ';'


#define SERCMDPARSER__SERIAL_BUFFER_SIZE 200

#define SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE 10
#define SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS 3


//====service lines, keep it here

//===this line is necessary to check if that file exists into scetch folder 
//===(because Arduino uses outdated C++ pre-processor version and cannot check if file exists)
#define CMD_PARSER_LOCALSETTINGS_OK
#define CMD_PARSER_VERSION 2

#endif // cmdParserV2Settings_h__