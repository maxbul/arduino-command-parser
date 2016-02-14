

#include "CommandParser.h"
//======================================
//
//======================================
void SerialCommandParser::addCommand(char parChar, const __FlashStringHelper * parDescription, void (*parFunction)(), uint8_t parNumberOfParameters){
	commandsArray[numberOfCommands].cmdChar=parChar;
	commandsArray[numberOfCommands].description=parDescription;
	commandsArray[numberOfCommands].function=parFunction;	
	commandsArray[numberOfCommands].numberOfParameters=parNumberOfParameters;	
	if (numberOfCommands>=SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE)
	{
		syntaxHalt(F("Array of commands is full"));
	}
	if (parNumberOfParameters>=SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS)
	{
		syntaxHalt(F("Too many parameters declared for function"));
	}
#ifdef 	DEBUG_SERIAL_PARSER
	Serial.print(F("Add command "));
	Serial.print(numberOfCommands);
	Serial.print(F(":\t char='"));
	Serial.write(commandsArray[numberOfCommands].cmdChar);
	Serial.print(F("',\tdescription:'"));
	Serial.print(commandsArray[numberOfCommands].description);
	Serial.println("'");
#endif  //\	DEBUG_SERIAL_PARSER
	
	numberOfCommands++;
}
//======================================
//
//======================================
void SerialCommandParser::syntaxHalt( const __FlashStringHelper * parErrorMessage){
	Serial.print(F("\n\rERROR:"));
	Serial.println(parErrorMessage);
	showDebugInfo();
	Serial.print(F("Device HALTED!\n\r"));
	while(1);
}
//======================================
//
//======================================
void SerialCommandParser::processParserError( const __FlashStringHelper * parErrorMessage){
	Serial.print(F("\n\rCMD.PARSER ERROR:"));
	Serial.print(parErrorMessage);
	Serial.print(F("\n\rCommand skipped!\n\r"));
	showDebugInfo();
	clearSerialCommand();
}
//======================================
//
//======================================
void SerialCommandParser::PrintCommandList(){
	Serial.print(F("-=List of commands ("));
	Serial.print(numberOfCommands);
	Serial.println(F("):=-"));
	for (int i = 0; i <numberOfCommands  ; i++)
	{
		Serial.write('\'');
		Serial.print(commandsArray[i].cmdChar);
		Serial.print("' :\t");
		Serial.println(commandsArray[i].description);
	}
	Serial.println(F("------"));
}
//======================================
//
//======================================
void SerialCommandParser::showDebugInfo(){
	Serial.print(F("\n\tcmd>'"));
	Serial.write(serialBuffer[0]);
	Serial.print(F("'\n\r\'"));
	if(currentParsedCommand){
		Serial.print(currentParsedCommand->description);
	}else{
		Serial.write('?');
	}
	Serial.print(F("'\n\r\tnumb.of param.:"));
	
	Serial.println(numberOfParameters);
	for (int i = 0; i < numberOfParameters ; i++)
	{
		Serial.print(F("\t\tparam. "));
		Serial.print(i);
		Serial.print(F("='"));
		Serial.print(parameters[i]);
		Serial.print(F("'\n\r"));
	}
}

//======================================
//
//======================================
void SerialCommandParser::parseSerialInput(){
	if (commandStringReceived == true){
			//previous command is not proceed yet, skip
	return;
	}
	while (Serial.available()) {
			//=== get the new byte...
			char inChar = Serial.read();
			
			#ifdef 	DEBUG_SERIAL_PARSER
			Serial.print(F("<<[0x"));
			Serial.print(inChar, HEX);
			Serial.write(']');
			Serial.write(inChar);
			Serial.print("\n\r'");
			Serial.print(serialBuffer);
			Serial.print("'\n\r");
			#endif  //\	DEBUG_SERIAL_PARSER			
			
			if(inputCursorPosition>=SERCMDPARSER__SERIAL_BUFFER_SIZE){
				//===maximum lenght of string is reached
					processParserError(F("Cmd.is too long, ignored!"));
					clearSerialCommand();
					Serial.flush();//clear input buffer (to lost too long string)
					return;
			} //\			if(inputCursorPosition>=SERCMDPARSER__SERIAL_BUFFER_SIZE){
			
			
			if(inChar=='\n' || inChar=='\r'){ //===new command received, parse it
				//\n or \r from prev. command is leaved
				if (inputCursorPosition==0)
				{
					continue;
				}
				
				commandStringReceived = true;
				
				
				#ifdef 	DEBUG_SERIAL_PARSER
				Serial.print(F("Input string = '"));
				Serial.print(serialBuffer);
				Serial.print(F("'\n\r"));
				#endif  //\	DEBUG_SERIAL_PARSER
				
				char commandChar=serialBuffer[0];
				
				for (size_t curCmdIndex = 0; curCmdIndex  <numberOfCommands  ; curCmdIndex ++)
				{
					if (commandsArray[curCmdIndex].cmdChar==commandChar)
					{
						currentParsedCommand=&commandsArray[curCmdIndex];
						break;
					}
				} //\for (int curCmdIndex = 0; curCmdIndex  <numberOfCommands  ; curCmdIndex ++)
				
				if (currentParsedCommand==NULL)
				{
					processParserError(F("\nUnknown command"));
					return;
				}else{
					//===command recognized
					
					Serial.write('\t');
					Serial.println(currentParsedCommand->description);
					//===parseParameter	
					bool awaitingNextParam=true;
					
					uint8_t tmpCursor;
					for (tmpCursor = 1; tmpCursor<=inputCursorPosition  ; tmpCursor++)
					{
						//===replace all spaces by null-symbols
						//if ((serialBuffer[tmpCursor]==' ') || (serialBuffer[tmpCursor]=='\n'))
						//==all non-printable symbols will be ignored
						if ((serialBuffer[tmpCursor]<33) || (serialBuffer[tmpCursor]>126))
						{
							serialBuffer[tmpCursor]='\0';
							awaitingNextParam=true; //cock a trigger

						}else if (awaitingNextParam==true){
							//===too many parameters found  - ignore the whole  command
							if (numberOfParameters>=currentParsedCommand->numberOfParameters){
								processParserError(F("Too many parameters"));
								return;
							}
							parameters[numberOfParameters++]=serialBuffer+tmpCursor;	
							awaitingNextParam=false; //clear trigger
						}
					}
					
					//===too few parameters found  - ignore the whole command
					if (numberOfParameters< currentParsedCommand->numberOfParameters){
						processParserError(F("Too few parameters"));
						return;
					}
					
					#ifdef 	DEBUG_SERIAL_PARSER
					showDebugInfo();
					#endif  //\	DEBUG_SERIAL_PARSER
					
					//===execute command
					currentParsedCommand->function();
					clearSerialCommand();
					return;
				} //\(!commandRecognized)
				
			}else{ //\if(inChar=='\n'){ //===new command received, parse it
				
				//just one more char received - add to buffer
				serialBuffer[inputCursorPosition++]=inChar;
				/*
				commandParameter=CMD_PARAMETER_NULLVALUE;
				size_t multiplier=1;
				
				for (int i = 1; i < COMMAND_BUFFER_LENGHT  ; i++)
				{
					//printf("i= %d par=%d char=%d\n", i, commandParameter, commandInputBuffer[i]);
					//===skip spaces between command and parameter
					if (commandParameter==CMD_PARAMETER_NULLVALUE){
						//printf("null\n");
						if (commandInputBuffer[i]==0x20){
							//printf("space\n");
							continue;
						}
					}
					//===end of line occured
					if (commandParameter=='\n'  || commandParameter=='\r')
					{
						//printf("end\n");
						break;
					}
					//===skip  after first non-numeric char occured
					if (commandInputBuffer[i]<'0' || commandInputBuffer[i] > '9')
					{
						//printf("done\n");
						break;
					} //\if (commandInputBuffer[i]<'0' || commandInputBuffer[i] > '9')
					//===here we have 'numeric' character - add that digit to parameter
					if (commandParameter==CMD_PARAMETER_NULLVALUE)
					{
						//convert first digit to integer
						commandParameter==commandInputBuffer[i]-'0';
						}else{
						//convert other digits
						commandParameter*=10;
						commandParameter+=commandInputBuffer[i]-'0';
					} //\if (commandParameter==CMD_PARAMETER_NULLVALUE)
				}
				//===clear input buffer and cursor
				memset(commandInputBuffer, 0, COMMAND_BUFFER_LENGHT);
				commandCurrentInputCursor=0;
				size_t pos=0;
				
				printf_P(PSTR("Current command = %c, parameter=%d\n"), commandChar, commandParameter);
				}else{
				//=== new char received  - add it to the input command string:
				commandInputBuffer[commandCurrentInputCursor++]=inChar;
			}
			*/	
		}//if(inChar=='\n'){ //===new command received, parse it
	
	} //\while (Serial.available()) {
	
}
//======================================
//
//======================================
void SerialCommandParser::clearSerialCommand(){
	memset(serialBuffer, 0, SERCMDPARSER__SERIAL_BUFFER_SIZE);
	for (int i = 0; i < SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS ; i++)
	{
		parameters[i]=NULL;	
	}
	numberOfParameters=0;
	inputCursorPosition=0;
	commandStringReceived=false;
	currentParsedCommand=NULL;
	
} //\ SerialCommandParser::clearSerialCommand()

//======================================
//constructor
//======================================
SerialCommandParser::SerialCommandParser(){
	clearSerialCommand();
}
//======================================
//
//======================================
void SerialCommandParser::checkParameterNoOrHalt(int paramParamNo){
	if ((paramParamNo+1) > currentParsedCommand->numberOfParameters)
	{
		syntaxHalt(F("Required param. No is too big\n"));
	}
}
//======================================
//
//======================================
int SerialCommandParser::getParameterInt(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);	
	return stringToInt(parameters[paramParamNo]);
}
//======================================
//
//======================================
char SerialCommandParser::getParameterChar(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);	
	return parameters[paramParamNo][0];
}
//======================================
//
//======================================
char *SerialCommandParser::getParameterString(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);
	return parameters[paramParamNo];
}

	
//======================================
int SerialCommandParser::stringToInt(char * parString){
		int result=0;
		char curChar;
		char curDigit;
		char aSign=1; //1 or -1
		size_t cursorPosition=0;
		while (parString[cursorPosition] != '\0')
		{
			curChar=parString[cursorPosition];
/*			
Serial.print("\r\t pos=");			
Serial.print(cursorPosition);
Serial.print("\t char=");
Serial.println(curChar);
*/
			curDigit=curChar-'0';
			if ((cursorPosition==0) && (curChar=='-')){
				//==first char is '-' - negative value
				aSign=-1;
				curDigit=0;
			}else if ((curDigit<0)||(curDigit>9)){
				//it's not a number (contains non-digital chars)
				return(SERCMDPARSER__INTEGER_NULLVALUE);
			}
			result*=10;
			result+= curDigit; //convert char (number) to binary digit value
			cursorPosition++;
		}
		result*=aSign;
		return result;
}
//======================================
//
//======================================
char SerialCommandParser::waitForSingleChar( const __FlashStringHelper * parMessage){
	//===clear buffers
	clearSerialCommand();
	Serial.flush();
	//print a message
	Serial.println(parMessage);
	Serial.println("Press desired key and press enter");
	
	//wait until any command
	while (!Serial.available()){
		
	};
	char result=Serial.read();
	
	//===clear buffers again
	clearSerialCommand();
	Serial.flush();
	return result;
};
	
	
