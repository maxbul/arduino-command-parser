//#include <mbDebug.h>
#include <MBCommandParser_v3.h>

#define LED_PIN 13
MBCommandParser_v3 commandParser(&Serial, &Serial);


//======================================
//test function 1
//======================================
void testHelloWorld(){
	Serial.println("\nHELLO WORLD ");
  int curParam=commandParser.getParameterInt(0);
  Serial.print("parameter =");
  Serial.println(curParam);
  
  if(curParam==0){
    Serial.print("LED OFF");
    digitalWrite(LED_PIN, LOW);
  }else{
    Serial.print("LED ON");
    digitalWrite(LED_PIN, HIGH);
  }
}

//======================================
//test function 2
//======================================
void test2(){
	Serial.println("\nTHIS IS A TEST 2");
	Serial.print("\t\tint 0=");
	Serial.print(commandParser.getParameterInt(0));
	Serial.print("\t\tint 1=");
	Serial.println(commandParser.getParameterInt(1));
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
  
	Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  
	commandParser.addCommand('?', F("show help (this screen)"), showHelp, 0);
	commandParser.addCommand('A', F("test function (print and toggle LED on/off"), testHelloWorld, 1);
	commandParser.addCommand('B', F("func2"), test2, 2);

	digitalWrite(LED_PIN, 1);
	commandParser.printOptionsSettings();
	commandParser.printCommandList();
}

//======================================
//
//======================================
void loop(){
	commandParser.loop();
}
