#include <mbDebug.h>
#include <MBCommandParser_v3.h>



//=======software serial part
#if 1     //input to soft.serial, output to serial
  #include <SoftSerial.h>
  #define SOFT_SERIAL_TX_PIN  10
  #define SOFT_SERIAL_RX_PIN A4
  SoftwareSerial SoftSer(SOFT_SERIAL_RX_PIN, SOFT_SERIAL_TX_PIN);

  //1=SOFT NORMAL input -> softdebug ->hard 
  //0=SOFT SWAPPED: input -> softdebug ->hard 
  #if 0 
    #define DEBUG_SERIAL_PORT Serial
    #define INPUT_SERIAL_PORT SoftSer
  #else 
    #define DEBUG_SERIAL_PORT SoftSer
    #define INPUT_SERIAL_PORT Serial
  #endif
  
  SerialCommandParser_v3 commandParser(&INPUT_SERIAL_PORT, &DEBUG_SERIAL_PORT);
  
#else //all communication through Hardware Serial
  SerialCommandParser_v3 commandParser(&Serial, &Serial);

  #define DEBUG_SERIAL_PORT Serial
  #define INPUT_SERIAL_PORT Serial

  #define SOFT_SERIAL_RX_PIN  " 0 (Hardware Serial)"
  #define SOFT_SERIAL_TX_PIN  " 1 (Hardware Serial)"
#endif



//======================================
//test function 1
//======================================
void testHelloWorld(){
	DEBUG_SERIAL_PORT.println("\nHELLO WORLD 1");
}

//======================================
//test function 2
//======================================
void test2(){
	DEBUG_SERIAL_PORT.println("\nTHIS IS A TEST 2");
	DEBUG_SERIAL_PORT.print("\t\tint 0=");
	DEBUG_SERIAL_PORT.print(commandParser.getParameterInt(0));
	DEBUG_SERIAL_PORT.print("\t\tint 1=");
	DEBUG_SERIAL_PORT.print(commandParser.getParameterInt(1));
}

//======================================
//test function 3 (show help messages)
//======================================
void showHelp(){
	commandParser.printCommandList();
}
//======================================
//
//======================================
void setup(){
  
	DEBUG_SERIAL_PORT.begin(115200);
  INPUT_SERIAL_PORT.begin(9600); //works only when you use SoftSerial

  DEBUG_SERIAL_PORT.println("It's a DEBUG port");
  INPUT_SERIAL_PORT.println("It's an INPUT (command console) port");
  
  pinMode(13, OUTPUT);
  
  DEBUG_SERIAL_PORT.print("Console Rx pin:");
  DEBUG_SERIAL_PORT.println(SOFT_SERIAL_RX_PIN);
  DEBUG_SERIAL_PORT.print("Console Tx pin:");
  DEBUG_SERIAL_PORT.println(SOFT_SERIAL_TX_PIN);
  
	commandParser.addCommand('h', F("show help (this screen)"), showHelp, 0);
	commandParser.addCommand('A', F("function 1"), testHelloWorld, 1);
	commandParser.addCommand('B', F("func2"), test2, 2);

	digitalWrite(13, 1);
	commandParser.printOptionsSettings();
	commandParser.printCommandList();
}

//======================================
//
//======================================
void loop(){
	commandParser.loop();
}
