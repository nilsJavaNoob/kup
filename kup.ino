
/*  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}
*/
//===============
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 6, 3, 7);
#define DEBUG 

//MODES
const int WELCOME = 0;
const int TAKEMONEY = 1;
const int FILL_CLIENT = 3;
const int SERVICE_FILLING = 4;
const int L2duration =30000;//filling duration/sec
const int L4duration =30000;//filling duration/sec-3 min
//====== input section ========
const int RDYBTNPIN = 4;//input pin btn- (when pipe in client)
const int L2 = 2;
const int L4 = 4;
//TODO const int PIPE_ON_BASE = ??? 
//====== Pin output section ========
const int PUMP2L_RELAY = 8 ;//  out pin relay-valve control 
const int PUMP4L_RELAY = 9 ;//  out pin relay-valve control

volatile int  mCounterPulse = 0;//money pulse counter
int moneySumm = 0;// money summ
int mode = WELCOME;//default
int volume = 0;
//=====const=============
//максимальное время приема купюры
unsigned long maxTime = 3000;
unsigned long prevTime = 0; 

    //SETUP//

void setup() {

// initialize the library with the numbers of the interface pins

lcd.begin(16, 2);
// Print a message to the LCD.
lcd.print("Settings ....");
  
  #ifdef  DEBUG    //
    Serial.begin(9600);
    Serial.println("= in setup... =");
  #endif  
  delay(1000);  
  
  pinMode(2,INPUT);
  
  pinMode(PUMP2L_RELAY,OUTPUT);
  pinMode(PUMP4L_RELAY,OUTPUT);
  setPin(PUMP2L_RELAY,HIGH);// pin 8  - relay off 
  setPin(PUMP4L_RELAY,HIGH);// pin 9  - relay off
	
  pinMode(RDYBTNPIN,INPUT);
	
  // interrupt pin for pulse from kupuro pin 2(ard proMini)
	attachInterrupt(0,countmPulse,FALLING);//LOW,RISING,FALLING,CHANGE,
 //setPin(2,LOW);
#ifdef  DEBUG    
    Serial.println("= setup done =");  
#endif    
}//end setup

void loop(){
	switch(mode){
	  case WELCOME: 
			welcomeScreen(); //0 - displays "insert your money" 
		break;
	  case TAKEMONEY: 
			takeMoneyScreen();// 1 -"you put... press btn when ready"
		break;
	  case FILL_CLIENT: //case 3 
			fillClientTank();//"filling ... done Thank'u..... go showmoney mode"
		break; 
   default: mode = WELCOME;
	} //switch
} //loop

// obrabotk INTERRUPTS BLOCK
void countmPulse(){
	detachInterrupt(0);
	if(mode==TAKEMONEY){
		mCounterPulse++;
		#ifdef  DEBUG 
			Serial.println("p");
		#endif 
		if(mCounterPulse==1)
			prevTime=millis();//get current millis qwantity
	} 
	attachInterrupt(0,countmPulse,FALLING);
}//void
 
//=========case WELCOME - 0 ==========
void welcomeScreen(){
  lcd.clear();
  lcd.print("Bablo davai");
	#ifdef DEBUG
  	Serial.println("in welcome screen");
    Serial.println("Insert money");
	#endif
	 
	 mode=TAKEMONEY;//TODO mode =
	 
	 #ifdef DEBUG
		Serial.print("go to TAKEMONEY mode");
		Serial.println(mode);
	 #endif
}//welcomeScreen

//===========case TAKEMONEY -1 =========
void takeMoneyScreen(){
//с момента приема купюры прошло > 3 сек?
  if(mCounterPulse > 0 && (millis()- prevTime > maxTime)){
	moneySumm += 10*mCounterPulse;
	mCounterPulse =0;
	lcd.clear();
    lcd.print("Prinyato - ");
    lcd.print(moneySumm);
    lcd.setCursor(0, 1);
    lcd.print("next or press btn");
    
	
	#ifdef DEBUG
	  Serial.println("Received" );
      Serial.println(moneySumm);
	  Serial.println("Insert next or");
	  Serial.println("Insert pipe and press Ready button");
	  #endif
    
  }
  //=================!!!!!!!!!!!!!!!!!==============
  if(debounce(RDYBTNPIN,HIGH)){//btn was pressed
    if(moneySumm == 50){//(moneySumm >= 50 AND moneySumm <100)
         volume = L2;// 2 litres;
		#ifdef DEBUG
			Serial.println("entering mode: FILLTANK");
		#endif     
         mode=FILL_CLIENT;//fillClientTank()
    }     
    else if(moneySumm == 100){//(moneySumm >= 100)
              volume = L4; //4 litres - 2200;//  
		 #ifdef DEBUG
			Serial.println("entering mode: FILLTANK");
		 #endif
         mode=FILL_CLIENT;//FillClienttank()
    }
              
    else{ //error back to welcome mode
     volume = 0; 
		 lcd.clear();
     lcd.setCursor(0, 1);
     lcd.print("Error .....");
		 #ifdef DEBUG
			 Serial.print("error....");
			 Serial.println("back to mode: WELCOME");
		 #endif
     delay(2000);  
     mode=WELCOME;//back to Welcome() 
    }//else
    moneySumm = 0;
  } //if      
}//void showMoneyScreen()


//case FILLTANK -3
void fillClientTank(){
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Naliv .....");
	Serial.println("in fillClient()");
	if(volume == 2L){
	   setPin(PUMP2L_RELAY,LOW);//pump on,valve closed   
    
    #ifdef DEBUG
      Serial.println("filling 2 Litres...");
    #endif
     
     delay(L2duration);//L2duration
     setPin(PUMP2L_RELAY,HIGH);//pump off
     lcd.clear();
     lcd.setCursor(0, 1);
     lcd.print("Ok-Vernite shlang.");
     delay(2000);
} 
   
	else if(volume == 4L){
    setPin(PUMP4L_RELAY,LOW);//pump on,valve closed
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Naliv...");
   
    #ifdef DEBUG
      Serial.println("filling 4 Litres...");
    #endif
    delay(L4duration);//wait L4duration seconds
      
    setPin(PUMP4L_RELAY,HIGH);//pump off
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Ok-Vernite shlang.");delay(2000);

	}

	#ifdef DEBUG
		Serial.println("done ...Entering mode WELCOME");
	#endif
    delay(500);
	mode = WELCOME;
}//void
//=============================
//==========routines()=======
void setPin(int pin, bool level){
	digitalWrite(pin,level);
 }
 
bool debounce(int pin, bool level){
   static long lastTime;
   static bool lastBtn = false; 
   //снимаем состояние порта 
   bool btn = digitalRead(pin) == level;
   
   if(btn != lastBtn){//button pressed
		lastBtn = btn;
		lastTime = millis();
   }else{
		if(millis() - lastTime >= 20)
			return btn;
   }
   return false;
}//debounce()
