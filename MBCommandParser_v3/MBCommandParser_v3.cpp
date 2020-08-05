//#include <mbDebug.h>
#include "MBCommandParser_v3.h"
#include <stdlib.h>
//======================================
//
//======================================
void MBCommandParser_v3::addCommand(char parChar, const __FlashStringHelper * parDescription, void (*parFunction)(), uint8_t parNumberOfParameters){
	
	
	if (_numberOfCommands>=SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE-1)
	{
		syntaxHalt(F("Array of commands is full"));
	}
	commandsArray[_numberOfCommands].cmdChar=parChar;
	commandsArray[_numberOfCommands].description=parDescription;
	commandsArray[_numberOfCommands].function=parFunction;
	commandsArray[_numberOfCommands].numberOfParameters=parNumberOfParameters;
	
	#ifdef 	SERIAL_PARSER_SHOW_DEBUG
	_debugSerial->print(F("Add command "));
	_debugSerial->print(_numberOfCommands);
	_debugSerial->print(F(":\t char='"));
	_debugSerial->write(commandsArray[_numberOfCommands].cmdChar);
	_debugSerial->print(F("',\tdescription:'"));
	_debugSerial->print(commandsArray[_numberOfCommands].description);
	_debugSerial->println(F("'"));
	#endif  //\	SERIAL_PARSER_SHOW_DEBUG
	
	if (parNumberOfParameters>=SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS && parNumberOfParameters!=SERCMDPARSER_WHOLE_LINE_AS_FIRST_PARAMETER)
	{
		syntaxHalt(F("Too many parameters declared for function"));
	}
	
	_numberOfCommands++;
}
//======================================
//
//======================================
void MBCommandParser_v3::syntaxHalt( const __FlashStringHelper * parErrorMessage){
	_debugSerial->print(F("\n\rERROR:"));
	_debugSerial->println(parErrorMessage);
	
	#ifdef SERIAL_PARSER_SHOW_MESSAGES
		showDebugInfo();
	#endif
	_debugSerial->print(F("Device HALTED!\n\r"));
	_serial->print(F("Device HALTED!\n\r"));
	while(1);
}
//======================================
//
//======================================
void MBCommandParser_v3::processParserError( const __FlashStringHelper * parErrorMessage){
	#ifdef SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->print(F("\n\rSERL.CMDs.PARSER ERROR:"));
		_debugSerial->print(parErrorMessage);
		_debugSerial->print(F("\n\rCommand skipped!\n\r"));
		showDebugInfo();
	#endif
	clearSerialCommand();
}

//======================================
//
//======================================
void MBCommandParser_v3::printOptionsSettings(){
	#ifdef SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->println(F("-\t-\t-SERIAL PARSER OPTIONS-\t-\t"));
		#ifdef  SERIAL_PARSER_SHOW_DEBUG
			_debugSerial->print(F("Command preffix char:"));
			_debugSerial->println(SERIAL_COMMAND_PREFIX_CHAR);
		#endif
		
		_debugSerial->print(F("Bufer size:"));
		_debugSerial->println(SERCMDPARSER__SERIAL_BUFFER_SIZE);
		
		_debugSerial->print(F("Max numb. of commands:"));
		_debugSerial->println(SERCMDPARSER__COMMAND_ARRAY_MAX_SIZE);
		
		_debugSerial->print(F("Max. numb of params."));
		_debugSerial->println(SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS);
		//!!!!ADD TIMEOUTS HERE SERCMDPARSER__INTERCHAR_TIMEOUT_MILLIS
	#endif
	

}

//======================================
//
//======================================
void MBCommandParser_v3::printCommandList(){
	if (_maxLockingTime!=0){
		_debugSerial->print(F("LOCKING TIME="));
		_debugSerial->println(_maxLockingTime);
	}
	_debugSerial->print(F("\n-= Available commands: =-\n"));
	for (size_t i = 0; i <_numberOfCommands  ; i++)
	{
		_debugSerial->write('\'');
		_debugSerial->print(commandsArray[i].cmdChar);
		_debugSerial->print(F("' :\t"));
		_debugSerial->print(commandsArray[i].description);
		_debugSerial->write('(');
		if (commandsArray[i].numberOfParameters==SERCMDPARSER_WHOLE_LINE_AS_FIRST_PARAMETER){
			_debugSerial->println(F("unlim.string as param.)"));
		}else{
			_debugSerial->print(commandsArray[i].numberOfParameters);
			_debugSerial->println(F(" param.)"));
		}
		
		
	}
	#ifdef USE_SERIAL_COMMAND_PREFIX
		_debugSerial->print(F("\nMUST USE CMD PREFIX: '"));
		_debugSerial->write(SERIAL_COMMAND_PREFIX_CHAR);
		_debugSerial->println(F("'"));
	#endif
	_debugSerial->println(F("------"));
}


void MBCommandParser_v3::showGeneralInfo(){
	#ifdef SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->println(F("SHOW MESSAGES:YES"));
	#else
		_debugSerial->println(F("SHOW MESSAGES:NO"));
	#endif
	
	#ifdef SERIAL_PARSER_SHOW_DEBUG
		_debugSerial->println(F("SHOW DEBUG:YES"));
	#else
		_debugSerial->println(F("SHOW DEBUG:NO"));
	#endif
	
	_debugSerial->println(F("TIMEOUTS:"));
	_debugSerial->print(F("interchar = "));
	_debugSerial->print(_maxInterCharTimeout);
	_debugSerial->print(F(", command="));
	_debugSerial->println(_maxCommandTimeout);
}

//======================================
//
//======================================

void MBCommandParser_v3::showDebugInfo(){
	#ifdef SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->print(F("\n\tcmd>'"));
		#ifdef USE_SERIAL_COMMAND_PREFIX
			_debugSerial->write(_serialBuffer[1]);
		#else
			_debugSerial->write(_serialBuffer[0]);
		#endif
		
		_debugSerial->print(F("'\n\r\'"));
		if(_currentParsedCommand){
			_debugSerial->print(_currentParsedCommand->description);
		//}else{
		//	_debugSerial->write('?');
		}
		_debugSerial->print(F("'\n\r\tnumb.of param.:"));
	
		_debugSerial->println(_numberOfParameters);
		for (int i = 0; i < _numberOfParameters ; i++)
		{
			_debugSerial->print(F("\t\tparam. "));
			_debugSerial->print(i);
			_debugSerial->print(F("='"));
			_debugSerial->print(parameters[i]);
			_debugSerial->print(F("'\n\r"));
		}
	#endif
}

//======================================
//
//======================================
void MBCommandParser_v3::processSerialReading(){
	unsigned long runUntil=millis()+_maxLockingTime;
	
	if (_commandStringReceived == true){
		//previous command is not proceed yet, skip
		return;
	}
	///!!!!! MAKE THAT CODE SWITCHABLE by preprocessor (check #defines for timeouts)
	if (	
		(_inputCursorPosition>0)
		&&
		(
			(
				(_maxInterCharTimeout>0)
				&&(millis()-_lastIncomingCharMillis > _maxInterCharTimeout)
			)
		||
			(
				(_maxCommandTimeout>0)
				&&(millis()-_lastCommandStartedMillis > _maxCommandTimeout)
			)
		)
	  ){
			//===if we here  - some part of data may be lost, so discard
			#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
				_debugSerial->print(F("[CMD PARSER]: timeout, discard input data"));
			#endif
			//clear serial buffer
			while (_serial->available()){
				_serial->read();
			}
			//---extra check for a slow serial port
			delay(2);
			while (_serial->available()){
				_serial->read();
				delay(2);
			}
			clearSerialCommand();
			return;
	}
	
	while (_serial->available()) {
		if (_maxInterCharTimeout>0){
			_lastIncomingCharMillis=millis();
		}
		
		if (_maxLockingTime!=0){
			if (millis()>runUntil){
				return;
			}
		}
		//=== get the new byte...
		char inChar = _serial->read();
//_debugSerial->write(inChar);
//_debugSerial->write('~');
		#ifdef 	SERIAL_PARSER_SHOW_DEBUG
			_debugSerial->print(F("\t["));
			_debugSerial->write('/');
			_debugSerial->print(_inputCursorPosition);
			if (_maxInterCharTimeout!=0){
				_debugSerial->write('/');
				_debugSerial->print(millis()-_lastIncomingCharMillis);
			}
			_debugSerial->print(F("]<{0x"));
			_debugSerial->print(inChar, HEX);
			_debugSerial->write('}');
			_debugSerial->write(inChar);          
      /*
			_debugSerial->print(F("\t\tbuf:'"));
			_debugSerial->print(serialBuffer);       
      */
			_debugSerial->print(F("'\n\r"));
		#endif  //\	SERIAL_PARSER_SHOW_DEBUG
		if(inChar=='\n' || inChar=='\r'){ //===new command received, stop readig and wait until parsed
			//\n or \r from prev. command is stays in buffer - just skip it
			if (_inputCursorPosition==0){
				continue;
			}
			_serialBuffer[_inputCursorPosition]=inChar;
			_commandStringReceived = true;
			return;
		}else{
			#ifdef USE_SERIAL_COMMAND_PREFIX
				if (inChar==SERIAL_COMMAND_PREFIX_CHAR){
					if (_inputCursorPosition!=0){
						clearSerialCommand();
//_debugSerial->write('#');						
					}
				}else{
					if (_inputCursorPosition==0){
						//ignore the whole string if no preffix arrived
//_debugSerial->write('@');
						continue;
					}
				}
			#endif
			
				
			_serialBuffer[_inputCursorPosition]=inChar;
			///!!!!! MAKE THAT CODE SWITCHABLE by preprocessor (check #defines for timeouts)
			if (_inputCursorPosition==0){
				_lastCommandStartedMillis=millis();
			}
			
			_inputCursorPosition++;
		}	//if(inChar=='\n'){ //===new command received, parse it
		if(_inputCursorPosition>=SERCMDPARSER__SERIAL_BUFFER_SIZE){
			//===maximum lenght of string is reached
			#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
			processParserError(F("Cmd.is too long, ignored!"));
			#endif
			clearSerialCommand();
			_serial->flush();//clear input buffer (to lost too long string)
			return;
		} //\			if(inputCursorPosition>=SERCMDPARSER__SERIAL_BUFFER_SIZE){
			
	} //\while (_serial->available()) {	
}
//======================================
//
//======================================
boolean MBCommandParser_v3::parseSerialCommand(){
	if (_commandStringReceived == false){
		//command is not received yet
		return false;
	}
	boolean result = parseBuffer(_serialBuffer, SERCMDPARSER__SERIAL_BUFFER_SIZE);
	clearSerialCommand();
	return result;
}
//======================================
//
//======================================
boolean MBCommandParser_v3::parseBuffer(char * parBuffer, size_t parSize){
	if (parSize==0){
		return false;
	}
	uint8_t prefixOffset=0;
	#ifdef 	SERIAL_PARSER_SHOW_DEBUG
		_debugSerial->print(F("[CMD PARSER]Input string = '"));
		_debugSerial->print(parBuffer);
		_debugSerial->print(F("'\n\r"));
	#endif  //\	SERIAL_PARSER_SHOW_DEBUG
	#ifdef USE_SERIAL_COMMAND_PREFIX
		//===?new command started (prefix received)
		if (parBuffer[0]==SERIAL_COMMAND_PREFIX_CHAR){
			prefixOffset++;
		}else{
			//just ignore all input
			processParserError(F("Ignored (no prefix)"));
			return false;
		} //\if (serialBuffer[0]==SERIAL_COMMAND_PREFIX_CHAR){
	#endif //\USE_SERIAL_COMMAND_PREFIX
	char commandChar=parBuffer[0+prefixOffset];

	//===search command char in the array of commands			
	for (size_t curCmdIndex = 0; curCmdIndex  <_numberOfCommands  ; curCmdIndex ++){
			if (commandsArray[curCmdIndex].cmdChar==commandChar){
				_currentParsedCommand=&commandsArray[curCmdIndex];
				break;
			}
	} //\for (int curCmdIndex = 0; curCmdIndex  <numberOfCommands  ; curCmdIndex ++)
	if (_currentParsedCommand==NULL){
		//===command for that char not found
		processParserError(F("\nUnknown command"));
		clearSerialCommand();
		return false;
	}
	//===command recognized
	#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->write('\t');
		_debugSerial->println(_currentParsedCommand->description);
	#endif
	/*
	if (_copyOfIncomingBuffer!=NULL){
		memcpy(_copyOfIncomingBuffer, parBuffer, _copyOfIncomingBufferLenght);
		_copyOfIncomingBuffer[_copyOfIncomingBufferLenght-1]=0;
	}
	*/
	uint8_t tmpCursor;
	if (_currentParsedCommand->numberOfParameters==SERCMDPARSER_WHOLE_LINE_AS_FIRST_PARAMETER){
		//=== for unlimited parameter line - juist store a buffer
		#if 0// 	#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->println(F("[Use the whole line as parameter]"));
		_debugSerial->print(F("[Parameter:'"));
		_debugSerial->println(&parBuffer[prefixOffset+2]);
		_debugSerial->println(F("']"));
		#endif
		_numberOfParameters=1;
		for (tmpCursor = prefixOffset+2; tmpCursor<=parSize; tmpCursor++){
			if ((parBuffer[tmpCursor]>33) && (parBuffer[tmpCursor]<126)){
				parameters[0]=&parBuffer[tmpCursor];
				//remove non-printable characters from the end of the string
				for (size_t i = tmpCursor; i <parSize  ; i++){
					if (parBuffer[i]<=33){
						parBuffer[i]=0;
					}
				}
				break;
			}
		}

	}else{
		//===parseParameter	
		bool awaitingNextParam=true;
		//for (tmpCursor = prefixOffset+1; tmpCursor<=_inputCursorPosition  ; tmpCursor++){
		for (tmpCursor = prefixOffset+1; tmpCursor<parSize	 ; tmpCursor++){
			//_debug("tmpCursor",tmpCursor);
			//===replace all spaces by null-symbols
			//if ((serialBuffer[tmpCursor]==' ') || (serialBuffer[tmpCursor]=='\n'))
			//==all non-printable symbols will be ignored
			
			///!!!!!!!!!!!!!!!!!!!!!!!!!! ??? ne povynno rabotat
			
		#if 0	
			#ifdef 	SERIAL_PARSER_SHOW_DEBUG
				char curChar=parBuffer[tmpCursor];
				_debugSerial->print(F("\t["));
				_debugSerial->write('/');
				_debugSerial->print(tmpCursor);
				_debugSerial->print(F("]<{0x"));
				_debugSerial->print(curChar, HEX);
				_debugSerial->write('}');
				_debugSerial->write(curChar);          
      			_debugSerial->print(F("\t\tbuf:'"));
				_debugSerial->print(serialBuffer);       
      
				_debugSerial->print(F("'\n\r"));
			#endif  //\	SERIAL_PARSER_SHOW_DEBUG
		#endif  

		if ((parBuffer[tmpCursor]<33) || (parBuffer[tmpCursor]>126)){
				parBuffer[tmpCursor]='\0';
				awaitingNextParam=true; //cock a trigger
		}else if (awaitingNextParam==true){
				//===seems a new parameter started
				if (_numberOfParameters>=_currentParsedCommand->numberOfParameters){
					//===too many parameters already found  - ignore the whole  command

					processParserError(F("Too many parameters"));
					clearSerialCommand();
					return false;
				}
				parameters[_numberOfParameters++]=parBuffer+tmpCursor;	
				awaitingNextParam=false; //clear trigger
		} //\if ((serialBuffer[tmpCursor]<33) |....
	} //\for (tmpCursor = prefixOffs ....
	
	if (	
			(_numberOfParameters< _currentParsedCommand->numberOfParameters) 
			&& (_currentParsedCommand->numberOfParameters!=SERCMDPARSER_WHOLE_LINE_AS_FIRST_PARAMETER)
		){
		//===too few parameters found  - ignore the whole command
		#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
			processParserError(F("Too few parameters"));
			
		#endif						
		clearSerialCommand();
		return false;
	} //\if (numberOfParameters< currentParsedCommand->numberOfParameters){
	
	}

	#ifdef 	SERIAL_PARSER_SHOW_DEBUG
		showDebugInfo();
	#endif  //\	SERIAL_PARSER_SHOW_DEBUG
					
	//===execute command
	_currentParsedCommand->function();
	clearSerialCommand();
	return true;
}

//======================================
//
//======================================
void MBCommandParser_v3::clearSerialInputBuffer(){
	#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
		_debugSerial->print(F("\n[Clear input buffer]"));
	#endif
	while (_serial->available())
	{
		_serial->read();
	}
	_serial->flush();	
}
	
//======================================
//
//======================================
void MBCommandParser_v3::clearSerialCommand(){
	memset(_serialBuffer, 0, min(_inputCursorPosition+1,(size_t)SERCMDPARSER__SERIAL_BUFFER_SIZE));
	for (int i = 0; i < SERCMDPARSER__MAX_NUMBER_OF_PARAMETERS ; i++)
	{
		parameters[i]=NULL;	
	}
	_numberOfParameters=0;
	_inputCursorPosition=0;
	_commandStringReceived=false;
	_currentParsedCommand=NULL;
	
} //\ SerialCommandParser_v3::clearSerialCommand()

//======================================
//constructor
//======================================
/* ---discarded, always use constructor with  2 parameters
SerialCommandParser_v3::SerialCommandParser_v3(Stream *parSerial){
	_serial=parSerial;
	_debugSerial=parSerial;
	clearSerialCommand();
}
*/
//======================================
//constructor
//======================================
MBCommandParser_v3::MBCommandParser_v3(Stream *parSerial, Stream *parDebugSerial){
	_serial=parSerial;
	//make that code switchable by #defines
	_maxInterCharTimeout=SERCMDPARSER__INTERCHAR_TIMEOUT_MILLIS;
	_maxCommandTimeout=SERCMDPARSER__COMMAND_TIMEOUT_MILLIS;
	
	clearSerialCommand();
	
	_debugSerial=parDebugSerial;
}
//======================================
//
//======================================
void MBCommandParser_v3::checkParameterNoOrHalt(int paramParamNo){
	#ifdef 	SERIAL_PARSER_SHOW_MESSAGES
	if ((paramParamNo) > _currentParsedCommand->numberOfParameters)
	{
		syntaxHalt(F("Requested numb. of params is too big for that command\nSee the function code of initialization of the command\n"));
	}
	#else
		//syntaxHalt(F("Wrong numb. of params"));
	#endif
}
//======================================
//
//======================================
long MBCommandParser_v3::getParameterInt(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);	
	return stringToInt(parameters[paramParamNo]);
}
//======================================
//
//======================================
float MBCommandParser_v3::getParameterFloat(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);
	return stringToFloat(parameters[paramParamNo]);
}
//======================================
//
//======================================
char MBCommandParser_v3::getParameterChar(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);	
	return parameters[paramParamNo][0];
}
//======================================
//
//======================================
char *MBCommandParser_v3::getParameterString(uint8_t paramParamNo){
	checkParameterNoOrHalt(paramParamNo);
	return parameters[paramParamNo];
}

	
//======================================
long MBCommandParser_v3::stringToInt(char  parString[]){
		long resultLong=0; //long
		char curChar;
		char curDigit;
		char aSign=1; //1 or -1
		size_t cursorPosition=0;
		while (parString[cursorPosition] != '\0')
		{
			curChar=parString[cursorPosition];
			
//_serial->print("\r\t pos=");			
//_serial->print(cursorPosition);
//_serial->print("\t char=");
//_serial->println(curChar);


			curDigit=curChar-'0';
			if ((cursorPosition==0) && (curChar=='-')){
				//==first char is '-' - negative value
				aSign=-1;
				curDigit=0;
			}else if ((curDigit<0)||(curDigit>9)){
				//it's not a number (contains non-digital chars)
				return(SERCMDPARSER__INTEGER_NULLVALUE);
			}
			resultLong*=10;
			resultLong+= curDigit; //convert char (number) to binary digit value
			cursorPosition++;
//_serial->print(F("Result="));
//_serial->println(resultLong);			
		}
		resultLong*=aSign;
		
		return resultLong;
}
//=========================================
//
//=========================================
float MBCommandParser_v3::stringToFloat(const char parString[]){
	return atof(parString);
}

//======================================
//
//======================================
char MBCommandParser_v3::waitForSingleChar( const __FlashStringHelper * parMessage){
	//===clear buffers
	clearSerialCommand();
	_serial->flush();
	//print a message
	_serial->println(parMessage);
	_serial->println(F("Press desired key and press enter"));
	
	//wait until any command
	while (!_serial->available()){
		
	};
	char result=_serial->read();
	
	//===clear buffers again
	clearSerialCommand();
	_serial->flush();
	return result;
};

//======================================
//
//======================================
void MBCommandParser_v3::setMaxLockingTime(unsigned long parMaxLockingTime){
	_maxLockingTime=parMaxLockingTime;
};
//======================================
//
//======================================
void MBCommandParser_v3::setIntercharTimeout(unsigned long parMaxTimeout){
	_maxInterCharTimeout=parMaxTimeout;
};

	
//======================================
//
//======================================
bool MBCommandParser_v3::waitForAKey( const __FlashStringHelper * parMessage, char parDesiredKey){
	//===clear buffers
	clearSerialCommand();
	_serial->flush();
	//print a message
	_serial->println(parMessage);
	_serial->println(F("Press '"));
	_serial->println(F("' key and then press Enter"));
	
	//wait until any command
	while (!_serial->available()){
		
	};
	char result=_serial->read();
	clearSerialCommand();
	_serial->flush();
	return (result==parDesiredKey);
}
//======================================
//
//======================================
uint8_t  MBCommandParser_v3::readStringToBuffer(	char parStringBuffer[]
														, uint8_t parMaxLenght
														, char parEndingChar
														, uint32_t parTimeout
													)
{
		char curChar;
		uint8_t curLenght=0;
		
		memset(parStringBuffer, 0, parMaxLenght);
		uint32_t maxTimeoutUntilMillis=millis()+parTimeout;
		while(1){
			if (millis()>maxTimeoutUntilMillis)	{
				processParserError(F("Timeout"));
				break;
			}
			if (_serial->available()){
				curChar=_serial->read();
				#ifdef 	DEBUG_PORT
					_debugSerial->write('~');
					_debugSerial->write(curChar);
				#endif
				if (curChar== parEndingChar){
					break;
				}
				parStringBuffer[curLenght]=curChar;
				curLenght++;
				if (curLenght> parMaxLenght-1){
					processParserError(F("String is too long, cutted."));
					clearSerialCommand();
					break;
				} 
			} //\if (_serial.available()){
		} //\while(1){
		return curLenght;
}//\uint8_t  SerialCommandParser_v3::readStringToBuffer(	

//=========================================
//
//=========================================
char * MBCommandParser_v3::getBuffer(){
	return _serialBuffer;

}
////======================================
////MAY BE NOT NECESSARY!
////======================================
//void MBCommandParser_v3::copyStringParameterToBuffer(uint8_t paramParamNo, char* parBuff, size_t parSizeOfBuffer){
//	memset(parBuff, 0, parSizeOfBuffer);
//	memcpy(parBuff, getParameterString(paramParamNo), parSizeOfBuffer);	
//}

//=========================================
//
//=========================================
/*
void MBCommandParser_v3::storeCopyOfIncomingBuffer(char * parCopyBuffer, uint8_t parBufferLenght){
	_copyOfIncomingBuffer=parCopyBuffer;
}
*/
//=========================================
//
//=========================================
void MBCommandParser_v3::loop(){
	processSerialReading();
	
	if(parseSerialCommand()){
		//just for debug purposes
	}else{
		//just for debug purposes
	};
}