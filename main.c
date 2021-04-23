#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#include "uart.h"

#define MAX_STRLEN 14	// minus 2 characters

// Global variables  --------------------
//volatile uint8_t tick=0,once=1;
char prijem[MAX_STRLEN];
//uint16_t impulzy=15535; // 50ms
uint16_t impulzy=49945; // 
volatile char nova_sprava=0;
unsigned int x=0;
//volatile unsigned char vysielaj0;
uint8_t casova_sprava[61]={0,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,1,0,0,1,0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0}; //2020-11-12 01:13:00 CET 
//time for test 2020-11-12 01:13:00 CET
unsigned int tick =0,znacka=12,tick0=0;
unsigned int minBCD[9]={0,0,0,0,0,0,0};
unsigned int hourBCD[8]={0,0,0,0,0,0,0};
unsigned int daycBCD[8]={0,0,0,0,0,0,0};
unsigned int dayBCD[4]={0,0,0};
unsigned int monthBCD[8]={0,0,0,0,0,0,0};
unsigned int yearBCD[9];
//---------------------------------------

//  Function prototypes ---------------
int timer_tick(int mode);
void port_setup();
void pwm_out();
int porovnaj_retazec(char *str1, char *str2, int pocet_znakov);
int over_spravu(char *vstup);
int dlzka_spravy(char *vstup);
void min_dec_to_BCD(int number);
void hour_dec_to_BCD(int number);
unsigned int day_cal_dec_to_BCD(int number);
unsigned int day_to_BCD(int number);
unsigned int month_to_BCD(int number);
unsigned int year_to_BCD(int number);
void dcf_out(int i);
//---------------------------------------

// Interrupt
ISR (TIMER0_OVF_vect){
	// citac/casovac0 - 8bit
 //TCNT0=0; 
 //if(tick0 == 260){PORTB ^= (1 << PB1);tick0 = 0;}	//100ms
 //if(tick0 == 520){PORTB ^= (1 << PB1);tick0 = 0;}	//200ms
 if(tick0 >= znacka){PORTB &= ~(1 << PB1); tick0=0;TCCR0B=0;}	//output
 else tick0++;
}


ISR (TIMER1_OVF_vect){
		// citac/casovac1 - 16bit
	  	// nastavenie poèiatoènej hodnoty poèítadla
	  	//TCNT1 = 3036; // 8sekund, pre delicku TCCR1B 1024 8MHz
  	TCNT1 = impulzy;	// 
		
	
	PORTB ^= (1 << PB2);
	PORTB ^= (1 << PB5); // set toggle
	if(tick < 60)dcf_out(tick); // minutova znacka - medzera
	tick++;
	if(tick >= 61)tick=0;
	//once = 1;
	
		// 	preteèenie registra TCNT1
		// 	preddelièku èítaèa/èasovaèa1 nastav na 256 (1/8MHz=0.125->0.125*256=32us)
		// 	1s/32us = 31250 impulzov,65536 – 31250 = 34286  
		//	1s/128us = 7812 impulzov, 65536 – 7812 = 57723
		//	2s/128us = 15625 imp, 65536 - 15625 = 49910
		//	5s/0.000128 = 39062, 65536 – 39062 = 26473 
		//	6s/128us =  46875,	65536 – 46875 = 18661
		//	8s/128us = 62500, 	65536 –62500 = 3036
		//	PORTD ^= (1 << PD7); //neguj PD7  
}  

ISR(USART_RX_vect){
	static int cnt=0,i;
	char prijaty_znak;
	volatile char prijate[MAX_STRLEN];	//docasna premenna
	
	prijaty_znak = usart_receive();
	
	//if(znak == 'T' ){zac=1;cnt=0;}
	if( (prijaty_znak != '\n') && ( prijaty_znak != '\r') && (cnt<MAX_STRLEN-2) ){
			prijate[cnt]=prijaty_znak;cnt++;
		}
		else {
			prijate[cnt]= '\0'; // koniec spravy osetrit znakom \0
			for(i=0;i<(cnt);i++)
				prijem[i]=prijate[i];
			//strncpy(nastavenie,prijem,MAX_STRLEN); 
			cnt=0;
			nova_sprava=1;
			//USART_flush();
			
			}

}


int main(void)
{
	int delicka=2,mod=2;
	uint8_t i,dlzka=2;
	char sprava[MAX_STRLEN],vysli[MAX_STRLEN];
	unsigned int parity_date=0;
	
	//TCCR0A |= (1 << WGM01); // CTC mode num.2
	TCCR0B |= (1 << CS02) | (1 << CS00); //preddelièka 1024 
    //OCR0A = 250;
    //TIMSK0 |= (1 << OCIE0A);   
	TIMSK0 |= (1 << TOIE0);
	//TCCR1B |= (1 << CS10)| (1 << CS11); // 64 prescaler
	//TIMSK1 |= (1 << TOIE1)|(1 << TOIE0);
	uartSetup();
	port_setup();
	delicka = timer_tick(mod);
	sei();
//	min_dec_to_BCD(40);
//	min_dec_to_BCD(44);
//	hour_dec_to_BCD(23);
//	hour_dec_to_BCD(11);
//	parity_date = day_cal_dec_to_BCD(21)^day_to_BCD(4)^month_to_BCD(10)^year_to_BCD(21);
	parity_date = parity_date;

	printf("pocet imp:%u\n",impulzy);
	
	
	printf("preddelicka:%i\n",delicka);
	while(1)
	 {
	 if(nova_sprava){
	  x = dlzka = dlzka_spravy(prijem);
	  printf_P(PSTR("Nacitane znaky %d\n\r"),x);
	  	for(i=0;i<dlzka;i++)
			{	sprava[i] = prijem[i];
			}
			
		//uartSendString("string");
		
		for(i=0;i<dlzka;i++)
			{	
				vysli[i] = sprava[i];
				//uartSendChar(vysli[i]);
				uartSendChar(vysli[i]);
			}
		switch( over_spravu(sprava) ){
		//if(sscanf_P(prijem,PSTR("imp=%d"),&x)){
		case 1: {	
			//if(  (10000*(vysli[6]-48) + 1000*(vysli[7]-48) + 100*(vysli[8]-48) + 10*(vysli[9]-48) ) < 65534 ){
			//	impulzy = ( 10000*(vysli[6]-48) + 1000*(vysli[7]-48) + 100*(vysli[8]-48) + 10*(vysli[9]-48) ); // ASCII kod 0-48, 1-49,2-50,
			if((10000*(sprava[6]-48) + 1000*(sprava[7]-48) + 100*(sprava[8]-48) + 10*(sprava[9]-48) ) < 65534){
				impulzy = ( 10000*(sprava[6]-48) + 1000*(sprava[7]-48) + 100*(sprava[8]-48) + 10*(sprava[9]-48) );
				}
				else {impulzy = impulzy;uartSendString(">65534\n");}
			printf_P(PSTR("Nacitane imp%u\n"),impulzy);
			}break;
		case 0: {	printf("0nespravny format, napr. **imp35534-->\n");
				printf("else prijata sprava->");
			}break;
		case 2: {
			//delicka=timer_tick( 1*(vysli[6]-48) );printf("preddelicka:%i\n",delicka);printf("1preddelicka:%i\n",(vysli[6]-48)); 
			delicka=timer_tick( 1*(sprava[6]-48) );printf("preddelicka:%i\n",delicka);printf("1preddelicka:%i\n",(sprava[6]-48)); 
			}break;
		case 3:{if(( (10*(sprava[5]-48) + (sprava[6]-48)) < 24) && ( (10*(sprava[5]-48) + (sprava[6]-48))>0 )){hour_dec_to_BCD(10*(sprava[5]-48) + (sprava[6]-48) );}
			}break;		
		case 4:{if(( (10*(sprava[5]-48) + (sprava[6]-48)) < 60) && ( (10*(sprava[5]-48) + (sprava[6]-48))>0 )){min_dec_to_BCD(10*(sprava[5]-48) + (sprava[6]-48) );}
			}break;	
		case 5:{if(( (10*(sprava[5]-48) + (sprava[6]-48)) < 32) && ( (10*(sprava[5]-48) + (sprava[6]-48))>0 )){day_cal_dec_to_BCD(10*(sprava[5]-48) + (sprava[6]-48) );}
			}break;
		case 6:{if(( (10*(sprava[5]-48) + (sprava[6]-48)) < 13) && ( (10*(sprava[5]-48) + (sprava[6]-48))>0 )){month_to_BCD(10*(sprava[5]-48) + (sprava[6]-48) );}
			}break;
		case 7:{if(( (10*(sprava[5]-48) + (sprava[6]-48)) < 60) && ( (10*(sprava[5]-48) + (sprava[6]-48))>0 )){year_to_BCD(10*(sprava[5]-48) + (sprava[6]-48) );}
			}break;
		case 8:{if( ( (sprava[5]-48 ) < 8) && ((sprava[5]-48)>0 ) ){day_to_BCD((sprava[5]-48) );}
			}break;
		default :	printf("nespravny format, napr. **imp35534, **hod13, **hod02-->\n");
		} // end of switch
		printf("\n");
		for(i=22;i<59;i++)
			{	
			printf(" %d",casova_sprava[i]);
			}		
		
		nova_sprava=0;
		}
	 
			 
	 }	// end of while
	return 0;
 
}	// end of main

int over_spravu(char *vstup)
{
	int vysledok=0;

	if (vstup[2]=='i' && vstup[3]=='m'  && vstup[4]=='p'){vysledok = 1;}
	if (vstup[2]=='d' && vstup[3]=='e'  && vstup[4]=='l'){vysledok = 2;} // prescaler
	if((vstup[2]!='i' && vstup[3]!='m'  && vstup[4]!='p') && (vstup[2]!='d' && vstup[3]!='e'  && vstup[4]!='l')) vysledok = 0;
	if(vstup[2]=='h' && vstup[3]=='o'  && vstup[4]=='d')vysledok = 3;	//hours
	if(vstup[2]=='m' && vstup[3]=='i'  && vstup[4]=='n')vysledok = 4;	//minutes
	if(vstup[2]=='d' && vstup[3]=='a'  && vstup[4]=='y')vysledok = 5;	//day date
	if(vstup[2]=='m' && vstup[3]=='o'  && vstup[4]=='n')vysledok = 6;	//month
	if(vstup[2]=='y' && vstup[3]=='e'  && vstup[4]=='a')vysledok = 7;	//year
	if(vstup[2]=='c' && vstup[3]=='d'  && vstup[4]=='a')vysledok = 8; // day of week
	return vysledok;
}

int dlzka_spravy(char *vstup)
{ 
	int len=0;

	while ((*vstup++)&&(len<MAX_STRLEN)){
		len++;
		}

	return len;

}

int timer_tick(int mode)
{
	int preddelicka = 2;
	
	TCCR1B = 0; 	//zastavenie citaca
	switch(mode) {
	case 1: {TCCR1B |= (1 << CS11);preddelicka=8; }break;// preddelicka 8
	case 2: {TCCR1B |= (1 << CS12) | (1 << CS10);preddelicka=1024; }break;//preddelièka 1024 (128us) 
	case 3: {TCCR1B |= (1 << CS12);preddelicka=256;}break; // preddelicka 256
	case 4: {TCCR1B |= (1 << CS10)| (1 << CS11);preddelicka=64;}break; // preddelicka 64
	case 5: {TCCR1B |= (1 << CS10);preddelicka=1;}break; // prescaler = 1
	default: {TCCR1B |= (1 << CS10)| (1 << CS11);preddelicka=64;}	
	}
	TIMSK1 |= (1 << TOIE1);  // prerušenie pri preteèení TCNT1     

    //OSCCAL = 0xA1;    // nastavenie kalibracneho bajtu interneho RC oscilatora
	return preddelicka;
}

void port_setup()
{
 DDRB |= (1 << PB1)|(1 << PB2);    //  ako výstupný 
 DDRB |= (1 << PB5);

 	//PORTB |= (1 << PB3);  	// on
	//PORTB &= ~(1 << PB3); 	// off
	//PORTB ^= (1<<PORTB3); 	//toggle
 	//DDRB &= ~(1 << PB1); // PB0 (ICP1) ako vstupny  
 PORTB |= (1 << PB1); //PB0 (ICP1) do log.1   
 PORTB |= (1 << PB5); // set high

}

void dcf_out(int i)
{
	//if(casova_sprava[i] == 1){PORTB |= (1 << PB1);} // twice
	// vynulovat a spustit
	if(casova_sprava[i] == 1){PORTB |= (1 << PB1);znacka = 11;TCNT0=0;TCCR0B |= (1 << CS02) | (1 << CS00);}	//520
	else {PORTB |= (1 << PB1);znacka = 5;TCNT0=0;TCCR0B |= (1 << CS02) | (1 << CS00);}	//260
	//if(casova_sprava[i] == 0){PORTB &= ~(1 << PB1);} //once
	//(casova_sprava[i] == 0)
}

void pwm_out()
{
	// set up PWM for LCD backlight control

	#define PWM_OUT OCR0B
	DDRD |= _BV( PD5);	// OC0B pin as output
	TCCR0A = _BV( WGM00) | _BV( WGM01) | _BV( COM0B1);	// Fast PWM, CTC+TOP
	TCCR0B = _BV( CS01);	// 1/8 prescale
}

int porovnaj_retazec(char *str1, char *str2, int pocet_znakov)
{	int i,rovnaky=0;
	
	for(i=0;i<pocet_znakov;i++){
		if(*str1++ == *str2++)rovnaky=0;
			else rovnaky = 1;
	}

return(rovnaky) ;
}



void min_dec_to_BCD(int number)
{
	unsigned int parity=0,i;
	//unsigned int temp=0;
	minBCD[8] = '\0';
	
	casova_sprava[28]=minBCD[6] = (number/40);
	casova_sprava[27]=minBCD[5] = ( (number-minBCD[6]*40)/20);
	casova_sprava[26]=minBCD[4] = ( (number-minBCD[6]*40)-(minBCD[5]*20) )/10;
	casova_sprava[25]=minBCD[3] = ( (number-minBCD[6]*40)-(minBCD[5]*20)-(minBCD[4]*10) )/8;
	casova_sprava[24]=minBCD[2] = ( (number-minBCD[6]*40)-(minBCD[5]*20)-(minBCD[4]*10)-(minBCD[3]*8) )/4;
	casova_sprava[23]=minBCD[1] = ( (number-minBCD[6]*40)-(minBCD[5]*20)-(minBCD[4]*10)-(minBCD[3]*8)-(minBCD[2]*4) )/2;
	casova_sprava[22]=minBCD[0] = ( (number-minBCD[6]*40)-(minBCD[5]*20)-(minBCD[4]*10)-(minBCD[3]*8)-(minBCD[2]*4)-(minBCD[1]*2) )/1;
	parity =  minBCD[0]^minBCD[1]^minBCD[2]^minBCD[3]^minBCD[4]^minBCD[5]^minBCD[6];
	casova_sprava[29]=minBCD[7] = parity;
	printf("BCD kod minuty %d ",number);
	for(i=0;i<8;i++)
		{	
		printf(" %d ",minBCD[i]);
		}
	uartSendChar('\n');
}


void hour_dec_to_BCD(int number)
{
	unsigned int parity=0,i;
	//unsigned int temp=0;
	hourBCD[7] = '\0';
	
	casova_sprava[35]=hourBCD[5] = (number/20);
	casova_sprava[34]=hourBCD[4] = (number-hourBCD[5]*20)/10;
	casova_sprava[33]=hourBCD[3] = ( (number-hourBCD[5]*20)-(hourBCD[4]*10) )/8;
	casova_sprava[32]=hourBCD[2] = ((number-hourBCD[5]*20)-(hourBCD[4]*10)-(hourBCD[3]*8) )/4;
	casova_sprava[31]=hourBCD[1] = ((number-hourBCD[5]*20)-(hourBCD[4]*10)-(hourBCD[3]*8)-(hourBCD[2]*4) )/2;
	casova_sprava[30]=hourBCD[0] = ((number-hourBCD[5]*20)-(hourBCD[4]*10)-(hourBCD[3]*8)-(hourBCD[2]*4)-(hourBCD[1]*2) )/1;
	parity =  hourBCD[0]^hourBCD[1]^hourBCD[2]^hourBCD[3]^hourBCD[4]^hourBCD[5];
	casova_sprava[36]=hourBCD[6] = parity;
	printf("BCD kod hodiny %d ",number);
	for(i=0;i<7;i++)
		{	
		printf(" %d ",hourBCD[i]);
		parity =  hourBCD[i]^hourBCD[i];
		}
	uartSendChar('\n');
}

unsigned int day_cal_dec_to_BCD(int number)
{	unsigned int parity=0;
	daycBCD[6] = '\0';

	casova_sprava[41]=daycBCD[5] = (number/20);
	casova_sprava[40]=daycBCD[4] = (number-daycBCD[5]*20)/10;
	casova_sprava[39]=daycBCD[3] = ( (number-daycBCD[5]*20)-daycBCD[4]*10 )/8;
	casova_sprava[38]=daycBCD[2] = ( (number-daycBCD[5]*20)-daycBCD[4]*10-daycBCD[3]*8 )/4;
	casova_sprava[37]=daycBCD[1] = ( (number-daycBCD[5]*20)-daycBCD[4]*10-daycBCD[3]*8-daycBCD[2]*4 )/2;
	daycBCD[0] = ( (number-daycBCD[5]*20)-daycBCD[4]*10-daycBCD[3]*8-daycBCD[2]*4-daycBCD[1]*2 )/1;
	casova_sprava[42]=parity =  daycBCD[0]^daycBCD[1]^daycBCD[2]^daycBCD[3]^daycBCD[4]^daycBCD[5];
	
	return parity;
}

unsigned int day_to_BCD(int number)
{	
	unsigned int parity=0;
	dayBCD[3] = '\0';

	casova_sprava[45]=dayBCD[2] = (number)/4;
	casova_sprava[44]=dayBCD[1] = (number-dayBCD[2]*4)/2;
	casova_sprava[43]=dayBCD[0] = ((number-dayBCD[2]*4)-(dayBCD[1]*2) )/1;
	parity =  dayBCD[0]^dayBCD[1]^dayBCD[2]^dayBCD[3];
	return parity;
}

unsigned int month_to_BCD(int number)
{
	unsigned int parity=0;
	monthBCD[5] = '\0';

	casova_sprava[50]=monthBCD[4] = (number/10);
	casova_sprava[49]=monthBCD[3] = (number-monthBCD[4]*10)/8;
	casova_sprava[48]=monthBCD[2] = ( (number-monthBCD[4]*10)-(monthBCD[3]*8) )/4;
	casova_sprava[47]=monthBCD[1] = ( (number-monthBCD[4]*10)-(monthBCD[3]*8)-(monthBCD[2]*4) )/2;
	casova_sprava[46]=monthBCD[0] = ( (number-monthBCD[4]*10)-(monthBCD[3]*8)-(monthBCD[2]*4)-(monthBCD[1]*2) )/1;
	parity =  monthBCD[0]^monthBCD[1]^monthBCD[2]^monthBCD[3]^monthBCD[4];
	return parity;
}

unsigned int year_to_BCD(int number)
{	unsigned int parity=0;
	yearBCD[8] = '\0';

	casova_sprava[58]=yearBCD[7] = (number/80);
	casova_sprava[57]=yearBCD[6] = (number-yearBCD[7]*80)/40;
	casova_sprava[56]=yearBCD[5] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40) )/20;
	casova_sprava[55]=yearBCD[4] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40)-(yearBCD[5]*20) )/10;
	casova_sprava[54]=yearBCD[3] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40)-(yearBCD[5]*20)-(yearBCD[4]*10) )/8;
	casova_sprava[53]=yearBCD[2] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40)-(yearBCD[5]*20)-(yearBCD[4]*10)-(yearBCD[3]*8) )/4;
	casova_sprava[52]=yearBCD[1] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40)-(yearBCD[5]*20)-(yearBCD[4]*10)-(yearBCD[3]*8)-(yearBCD[2]*4) )/2;
	casova_sprava[51]=yearBCD[0] = ( (number-yearBCD[7]*80)-(yearBCD[6]*40)-(yearBCD[5]*20)-(yearBCD[4]*10)-(yearBCD[3]*8)-(yearBCD[2]*4)-(yearBCD[1]*2) )/1;
	parity =  yearBCD[0]^yearBCD[1]^yearBCD[2]^yearBCD[3]^yearBCD[4]^yearBCD[5]^yearBCD[6]^yearBCD[7];
	return parity;
}
