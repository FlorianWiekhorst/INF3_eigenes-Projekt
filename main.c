#include <lpc11xx.h>
#include "spi.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int button;		// der Button1
unsigned int button2;		// der button2
int button_count = 0;		// Button_press_timer
int button2_count = 0;		// Button_press_timer
int move_timer = 0;		// to time the movement
int running = 1;		// 0 == false; 1 == true;
int rundenAnzahl = 0;		// Zähler für bestandene Runden
int zufallszahl = 1;
int guterBlock = 1;		// bestimmt welcher Block passierbar ist / aendert collisionsBereiche
int wallStep = 0;		// zählt schritte der Wand, ermoeglicht GameFreeze

int x; //globale Variable für die Schleife (komplett einfaerben)
int v; //globale Variable für die zweite schleife (Player darstellen/faerben)

// 4 FarbArrays für die 4 colorWände
int farbe_1 = 6;  // Farbvariable für Wall 1
int farbe_2 = 6;  // Farbvariable für Wall 2
int farbe_3 = 6;  // Farbvariable für Wall 3  // Alle in der ersten Runde Weiss!
int farbe_4 = 6;  // Farbvariable für Wall 4
// Eine FarbWahl Variable für den Player
int farbWahl = 24;


const unsigned char FarbTopf_1[7] =		//FarbArray für colorWall 1
{
   0x22, 0xEC, 0x11, 0x80, 0x51, 0xB8, 0xFF
};
const unsigned char FarbTopf_2[7] =		//FarbArray für colorWall 2
{
   0xF0, 0xA1, 0x45, 0x53 , 0x20, 0x3D ,0xFF
};
const unsigned char FarbTopf_3[7] =		//FarbArray für colorWall 3
{
   0xD4, 0x4C, 0x37, 0xA9, 0x91, 0x72 ,0xFF
};
const unsigned char FarbTopf_4[7] =		//FarbArray für colorWall 4
{
   0x29, 0xD6, 0x42, 0xCC, 0x65, 0xF2 ,0xFF
};
const unsigned char FarbTopf_Player[25] =	//FarbArray für Player - alle 16 Farben
{
   0x22, 0xEC, 0x11, 0x80, 0xF0, 0xA1, 0x45, 0x53, 0xD4, 0x4C, 0x37, 0xA9 , 0x29, 0xD6, 0x42, 0xCC,
   0x51, 0xB8, 0x20, 0x3D, 0x91, 0x72, 0x65, 0xF5, 0xFF
};

unsigned short player[] =			// Initialisierung vom player
	{
		0xEF08,	0x1800,	0x1237, 0x154A,	0x130F, // initialisierung, Ausrichtung, x1, x2, y1, y2
		0x1622
	};

unsigned short colorWall_1 [] =		//Initialisierung Farbwand
{
	0xEF08,	0x1800, 0x1200 , 0x151F ,0x1397 ,0x16AF
};

unsigned short colorWall_2 [] =		//Initialisierung Farbwand
{
	0xEF08,	0x1800, 0x1220 , 0x1540 ,0x1397 ,0x16AF
};

unsigned short colorWall_3 [] =		//Initialisierung Farbwand
{
	0xEF08,	0x1800, 0x1241 , 0x1561 ,0x1397 ,0x16AF
};

unsigned short colorWall_4 [] =		//Initialisierung Farbwand
{
	0xEF08,	0x1800, 0x1262 , 0x1583 ,0x1397 ,0x16AF
};


void Waitms(const unsigned int msWait) {
  unsigned int   aktTime, diff;

  aktTime = LPC_TMR32B0->TC; 		//aktuellen Zaehlerstand des Timers auslesen

  do {
					 //bereits vergangene Zeit berechnen
			if (LPC_TMR32B0->TC >= aktTime) diff = LPC_TMR32B0->TC - aktTime;
			else diff = (0xFFFFFFFF - aktTime) + LPC_TMR32B0->TC;
  } while (diff	< msWait);
}



/* ----------------------------------------------------------------
   Senden einer Sequenz von  16-Bit Kommandos an das TFT-Display
   mittels SPI-Interface. Dabei wird die Data/Command Leitung
   entsprechend gesetzt
   ---------------------------------------------------------------- */
void SendCommandSeq(const unsigned short * data, int Anzahl)
{
  int  index;
	unsigned char   SendeByte;

	for (index=0; index<Anzahl; index++)
	{
		LPC_GPIO0->DATA |= 0x10;		  //Data/Command auf High => Kommando-Modus
			
		SendeByte = (data[index] >> 8) & 0xFF; 	  //High-Byte des Kommandos
		SPISend8Bit(SendeByte);

		SendeByte = data[index] & 0xFF; 	  //Low-Byte des Kommandos
		SPISend8Bit(SendeByte);

		LPC_GPIO0->DATA &= ~0x10;	          //Data/Command auf Low => Daten-Modus	
	}
}


/* ----------------------------------------------------------------
   Initialisierung des TFT-Displays fuer den 65536-Farben Modus
   entsprechend der in der Vorlesung beschriebenen Schritte einschließlich
   der Uebertragung der Initialisierungs-Kommandos.
   Zur Ueberwachung der Zeitabstaende wird die Funktion Waitms verwendet.
   Zum senden eines Kommandos an das Display wird die Funktion SendCommand
   verwendet. 
   ---------------------------------------------------------------- */
void InitDisplay(void) {
	
    const unsigned short InitData[] = 
	{ 
	  //Initialisierungsdaten fuer 256 Farben Modus
		0xFDFD, 0xFDFD, 
		/* pause  */
		0xEF00, 0xEE04, 0x1B04, 0xFEFE, 0xFEFE, 
		0xEF90, 0x4A04, 0x7F1F, 0xEE04, 0x4306,  	// 1f in [7] für 256 Colors
		/* pause  */
		0xEF90, 0x0983, 0x0800, 0x0BAF, 0x0A00, 
		0x0500, 0x0600, 0x0700, 0xEF00, 0xEE0C, 
		0xEF90, 0x0080, 0xEFB0, 0x4902, 0xEF00, 
		0x7F01, 0xE181, 0xE202, 0xE276, 0xE183, 
		0x8001, 0xEF90, 0x0000,
		// pause
		0xEF08,	0x1800,	0x1200, 0x1583,	0x1300,
		0x16AF 	//Hochformat 132 x 176 Pixel 
	}; 
	
	

	Waitms(300); 
	LPC_GPIO1->DATA &= ~0x80;  //Reset-Eingang des Displays auf Low => Hardware-Reset;
    Waitms(75);
	LPC_GPIO2->DATA |= 0x400;  //SSEL auf High
    Waitms(75);
	LPC_GPIO0->DATA |= 0x10;   //Data/Command auf High
    Waitms(75);
	LPC_GPIO1->DATA |= 0x80;   //Reset-Eingang des Displays auf High => kein Hardware-Reset 
    Waitms(75); 
	
  SendCommandSeq(&InitData[0], 2);
    Waitms(75); 
  SendCommandSeq(&InitData[2], 10);
    Waitms(75); 
  SendCommandSeq(&InitData[12], 23);
		Waitms(75); 
  SendCommandSeq(&InitData[35], 6); 
}


int main() {

	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);	//siehe Hinweis in Getting Started
	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);		//Enables clock for 32-bit counter/timer 0
	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);		//Enables Clock for 16-bit-counter/timer0, s. [3.5.14]
  
   LPC_GPIO0->DIR	|= 0x10;	//PIO0_4 als Data/Command, digitaler Ausgang
	 LPC_GPIO1->DIR |= 0x80;	//PIO1_7 als Reset, digitaler Ausgang
	 LPC_GPIO2->DIR |= 0x400;       //PIO2_10 als SSEL, digitaler Ausgang
	 LPC_GPIO3->DIR |= 0x3F;        // PIO3_0 bis PIO3_5 sind Ausgänge
	
	 	
   //Timer32B0 initialisieren. Er liefert die Zeitbasis fuer die Funktion waitms
   LPC_TMR32B0->PR  = 48000; 	//bei P-Clock 48Mhz ergibt sich 1Khz Timer Auflösung
   LPC_TMR32B0->TCR = 0x02;  	//setzt Timer zurueck und haelt ihn an
   LPC_TMR32B0->TCR = 0x01;  	//startet Timer
	 LPC_GPIO2->IS |= 0x00;
	 LPC_GPIO2->IE |= 0x200;	// Random_IR
	 LPC_GPIO2->IEV |= 0x00;
	 NVIC->ISER[0] |=0x20000000;

	 //Timer16BO initialisieren. Er liefert die Zeitbasis zum entprellen unserer Buttons.
	 LPC_TMR16B0->PR = 48000;	  	//Prescaler setzten für Milisekunden
	 LPC_TMR16B0->TCR = 0x02;	 	//Timer 16B0 reset und anhalten
	 LPC_TMR16B0->MR0 = 1;		        //Match-Register0 = Jede Milisekunde
	 LPC_TMR16B0->MCR = 0x03;	  	//Match-Controll-Register IR und Reset durch MR0
	 LPC_TMR16B0->TCR = 0x1;		//Timer16B0 starten
	 NVIC->ISER[0] |= 0x10000;		//Interrupt Set Enable für Exception 16, Timer16_0, erlaubt Interrupts
	
	 //SPI-Schnittstelle initialisieren:
   SPIInit8BitMaster();
   
	 //Display initialisieren:
   InitDisplay();
	 
	 for(x = 0; x<23232; x++){  // komplettes TFT-Display faerben
		SPISend8Bit(0xFF);
	 }
	
	 SendCommandSeq(&player[0],6); // player erzeugen
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(FarbTopf_Player[farbWahl]);
					 }
	 
	 // Ab hier Farbwände
	 SendCommandSeq(&colorWall_1[0],6); // colorWall erzeugen
			for(v = 0; v<800; v++)	{	SPISend8Bit(FarbTopf_1[farbe_1]); }
		
		SendCommandSeq(&colorWall_2[0],6); // colorWall erzeugen
			for(v = 0; v<825; v++)	{	SPISend8Bit(FarbTopf_2[farbe_2]); }
					
		SendCommandSeq(&colorWall_3[0],6); // colorWall erzeugen
			for(v = 0; v<825; v++)	{	SPISend8Bit(FarbTopf_3[farbe_3]); }
					
		 SendCommandSeq(&colorWall_4[0],6); // colorWall erzeugen
			for(v = 0; v<850; v++)	{	SPISend8Bit(FarbTopf_4[farbe_4]); }
							 
	 while (1) {;} 	//Endlosschleife
}

		// Random Number Generator (RNG)
		void PIOINT2_IRQHandler(void){
			unsigned int i;
			NVIC->ICPR[0] |=0x20000000;
			i = LPC_TMR32B0->TC % 4 + 1;
			
			if(i == 1){
					zufallszahl=1;
			}
			else if(i == 2){
					zufallszahl=2;
			}
			else if(i == 3){
					zufallszahl=3;
			}
			else if(i == 4){
					zufallszahl=4;
			}
			LPC_GPIO2->IC = 0x200;
			return;
		}


		void TIMER16_0_IRQHandler(void){		        //Exception_Handler 

			 // button1
			button = LPC_GPIO2->DATA;		        // GPIO2DATA aufnehmen	
			button = (button & 0x200)>>9;			// Button maskieren  PIO2_9
			//  button2
			button2 = LPC_GPIO1->DATA;
			button2 = (button2 & 0x10)>>4;			//button2 maskieren PIO1_4
			
	

			// Button 2 links ausweichen (button welcher weiter weg vom Stromanschluss des Boardes ist.)
			if (running > 0){	// Moeglichkeit Buttons auszuschalten , wenn running = 0 gesetzt wird. nach Crash zb.
			if (!button2) {
					button2_count++;
					}
						else 	{
						button2_count =0;
						}
						if (button2_count == 6){							// nur wenn Button länger gedrückt wird - entprellung
							SendCommandSeq(&player[0],6);			// ursprungskaestchen uebermalen / verschwinden lassen
							for(v = 0; v<400; v++)
								{
									SPISend8Bit(0xFF);
								 }
								
							if (player[2]>0x1200){							// solange Kaestchen noch nicht am Rand
								player[2]--;       							// Kaestchen ein pixel naeher an den Rand
								player[3]--; 
							}
									SendCommandSeq(&player[0],6);	// neu positioniertes Kaestchen in rot malen
									for(v = 0; v<400; v++)
									{
										
										SPISend8Bit(FarbTopf_Player[farbWahl]);
									}
								button2_count =0;
						}
			
		
		
		
		// Button 1
		if (!button) {
		button_count++;
		
		}
			else 	{
			button_count =0;
			}
			if (button_count == 6){					// identisch mit button2 :)
			SendCommandSeq(&player[0],6);
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(0xFF);
					 }
				if (player[2]<0x1270){				// halt arraywerte erhoehen, statt verringern
				player[2]++;       
				player[3]++; 
				}
			SendCommandSeq(&player[0],6);
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(FarbTopf_Player[farbWahl]);
					}
			
		button_count =0;
		}
			
	
		// Bewegung der color_Wände
		move_timer++;															// move Timer zählt durchgehend hoch
		if (move_timer == 10){ 					/ Sobald Timer > 50
			
					// WALL 1
					SendCommandSeq(&colorWall_1[0],6);  // colorWall_1 alte position loeschen
					for(v = 0; v<800; v++)		    // alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen ( löschen)
					 }
					 colorWall_1[4]--;									// Blockposition zum oberen Rand hin verringern.
					 colorWall_1[5]--;	
				  SendCommandSeq(&colorWall_1[0],6); 		// colorWall_1 an neuer Position malen
					for(v = 0; v<800; v++)
					{
						SPISend8Bit(FarbTopf_1[farbe_1]);
					}
					
					// WALL 2
					SendCommandSeq(&colorWall_2[0],6);  // colorWall_1 alte position loeschen
					for(v = 0; v<825; v++)		    // alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen ( löschen)
					 }
					 colorWall_2[4]--;									// Blockposition zum oberen Rand hin verringern.
					 colorWall_2[5]--;	
				  SendCommandSeq(&colorWall_2[0],6); 		// colorWall_2 an neuer Position malen
					for(v = 0; v<825; v++)
					{
						SPISend8Bit(FarbTopf_2[farbe_2]);
					}

					// WALL 3
					SendCommandSeq(&colorWall_3[0],6);  // colorWall_3 alte position loeschen
					for(v = 0; v<825; v++)		    // alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen ( löschen)
					 }
					 colorWall_3[4]--;									// Blockposition zum oberen Rand hin verringern.
					 colorWall_3[5]--;	
				  SendCommandSeq(&colorWall_3[0],6); 		// colorWall_3 an neuer Position malen
					for(v = 0; v<825; v++)
					{
						SPISend8Bit(FarbTopf_3[farbe_3]);
					}	

					// WALL 4
					SendCommandSeq(&colorWall_4[0],6);  // colorWall_4 alte position loeschen
					for(v = 0; v<850; v++)		    // alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen ( löschen)
					 }
					 colorWall_4[4]--;									// Blockposition zum oberen Rand hin verringern.
					 colorWall_4[5]--;	
					 wallStep++;
				  SendCommandSeq(&colorWall_4[0],6); 		// colorWall_4 an neuer Position malen
					for(v = 0; v<850; v++)
					{
						SPISend8Bit(FarbTopf_4[farbe_4]);
					}					
					
					
					SendCommandSeq(&player[0],6); // player erzeugen damit dieser nicht übermalt wird
						for(v = 0; v<400; v++)
					{
						SPISend8Bit(FarbTopf_Player[farbWahl]);
					 }				 
					move_timer = 0;											// den Move timer reseten, damit er wieder auf 50 zählen kann
				}
		
				
				
				// COLLISONSABFRAGE!
				if(rundenAnzahl>0){    // Damit es nicht in der ersten "MatschRunde" freezed.
					if(guterBlock ==1 ){
						if(wallStep>115 && player[3] > colorWall_1[3]) {
							running=0;
						}
					}
					
						if(guterBlock ==4){
						if(wallStep>115 && player[2] < colorWall_4[2]) {
							running=0;
						}
					}
					
						if(guterBlock ==2){
						if((wallStep>115 && player[2] < colorWall_2[2]) || (wallStep>115 && player[3] > colorWall_2[3])) {
							running=0;
						}
					}
					
					if(guterBlock ==3){
						if((wallStep>115 && player[2] < colorWall_3[2]) || (wallStep>115 && player[3] > colorWall_3[3])) {
							running=0;
						}
					}
			}
				
				
				
				
				
				// Alle color Walls neu positionieren: ENDE EINER RUNDE!
					if(colorWall_1[4]==0x1300){       	// Wenn Block1 oberen Rand erreicht
							running =0;								    	// spiel kurz einfrieren
							rundenAnzahl++;						    	// Eine Runde zur RundenAnzahl hinzufügen
						
						//Alle 4 colorWände weiß malen:
					SendCommandSeq(&colorWall_4[0],6);  // colorWall_4 alte position loeschen
					for(v = 0; v<850; v++)							// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen (loeschen)
					 }
					
					 // WALL 3
					SendCommandSeq(&colorWall_3[0],6);  // colorWall_3 alte position loeschen
					for(v = 0; v<825; v++)							// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen (loeschen)
					 }
					 
					 // WALL 2
					SendCommandSeq(&colorWall_2[0],6);  // colorWall_1 alte position loeschen
					for(v = 0; v<825; v++)							// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen (loeschen)
					 }
					 
					 // WALL 1
					SendCommandSeq(&colorWall_1[0],6);  // colorWall_1 alte position loeschen
					for(v = 0; v<800; v++)							// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// mit Hintergrundfarbe füllen (loeschen)
					 }
					 SendCommandSeq(&player[0],6); // player nochmal malen, damit er nicht halbiert dargestelt wird
						for(v = 0; v<400; v++)
					{
						
						SPISend8Bit(FarbTopf_Player[farbWahl]);
					 }
					 
					 
							move_timer = 0;							//Timer move reset
						
							colorWall_1[2]=0x1200;			// alle color Wände an Anfangsposition
							colorWall_1[3]=0x151F;
							colorWall_1[4]=0x1397;
							colorWall_1[5]=0x16AF;
							
							colorWall_2[2]=0x1220;
							colorWall_2[3]=0x1540;
							colorWall_2[4]=0x1397;
							colorWall_2[5]=0x16AF;
					
						  colorWall_3[2]=0x1241;
							colorWall_3[3]=0x1561;
							colorWall_3[4]=0x1397;
							colorWall_3[5]=0x16AF;
						
						  colorWall_4[2]=0x1262;
							colorWall_4[3]=0x1583;
							colorWall_4[4]=0x1397;
							colorWall_4[5]=0x16AF;

							
							//Wände wählen jeweils eine eigene neue Farbe		
							farbe_1++;		
							if (farbe_1 > 5){ 
									farbe_1 = 0; 
							}
							
							farbe_2--;
							if (farbe_2 < 0){ 
									farbe_2 = 5; 
							}
							
							farbe_3--;
							if (farbe_3 < 0){ 
									farbe_3 = 5; 
							}
							
							farbe_4++;
							if (farbe_4 > 5){ 
									farbe_4 = 0; 
							}
							
							
							// 	Player sucht sich eine der neuen WandFarben als seine Farbe aus anhand von RNG
							if (zufallszahl == 1) {
									farbWahl+=2;
											if (farbWahl > 23)	{farbWahl=0;}
								}	
								if (zufallszahl == 2) {
									farbWahl+=4;
									    if (farbWahl > 23)	{farbWahl=1;}
								}	
								if (zufallszahl == 3) {
									farbWahl-=1;
									    if (farbWahl < 0)	{farbWahl=23;}
								}	
								if (zufallszahl == 4) {
									farbWahl-=2;
									    if (farbWahl < 0)	{farbWahl=22;}
								}	
								
							
							// Farbe neu bestimmen, wenn nicht mit einem Block identisch	
							while (FarbTopf_Player[farbWahl] != FarbTopf_1[farbe_1]  
											&& FarbTopf_Player[farbWahl] != FarbTopf_2[farbe_2]		
											&& FarbTopf_Player[farbWahl] != FarbTopf_3[farbe_3]
											&& FarbTopf_Player[farbWahl] != FarbTopf_4[farbe_4])
										{
											farbWahl++;
										}
					
							
							
						if(FarbTopf_Player[farbWahl] == FarbTopf_1[farbe_1]) // Block 1 ist der Gute
							{
								guterBlock =1;
							}
						if(FarbTopf_Player[farbWahl] == FarbTopf_2[farbe_2]) // Block 2 ist der Gute
							{
								guterBlock =2;
							}
						if(FarbTopf_Player[farbWahl] == FarbTopf_3[farbe_3]) // Block 3 ist der Gute
							{
								guterBlock =3;
							}
						if(FarbTopf_Player[farbWahl] == FarbTopf_4[farbe_4]) // Block 4 ist der Gute
							{
								guterBlock =4;
							}
		
							
							Waitms(50);
							wallStep = 0;           // reset wallStepCounter :)
							running=1;	        // auftauen
					}
				
	LPC_TMR16B0->IR = 0x1;		//IR quittieren für MR0, beenden des interrupts
		return;	
	}  
}
