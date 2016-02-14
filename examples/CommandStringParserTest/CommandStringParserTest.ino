#include <CommandParser.h>

SerialCommandParser commandParser;

//======================================
//test function 1
//======================================
void testHelloWorld(){
	Serial.println("\nHELLO WORLD 1");
}

//======================================
//test function 2
//======================================
void test2(){
	Serial.println("\nTHIS IS A TEST 2");
	Serial.print("\t\tint 0=");
	Serial.print(commandParser.getParameterInt(0));
	Serial.print("\t\tint 1=");
	Serial.print(commandParser.getParameterInt(1));
	Serial.print("\t\tint 2=");
	Serial.print(commandParser.getParameterInt(2));
}

//======================================
//test function 3 (show help messages)
//======================================
void showHelp(){
	commandParser.PrintCommandList();
}
//======================================
//
//======================================
void setup(){
	Serial.begin(9600);
	
	pinMode(13, OUTPUT);
	
	commandParser.addCommand('h', F("show help (this screen)"), showHelp, 0);
	commandParser.addCommand('b', F("function 1"), testHelloWorld, 1);
	commandParser.addCommand('a', F("func2"), test2, 2);

	digitalWrite(13, 1);
	commandParser.PrintCommandList();
}

//======================================
//
//======================================
void loop(){
	commandParser.parseSerialInput();
}
