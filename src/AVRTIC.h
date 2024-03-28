#include<Arduino.h>


#define maxJobQueue 8

/*
 * En meget crap måde at styre konkurrente systemer på i arduino.... ja FUck siger jeg bare
 * 
 * Vi kører en ticksize sat af timer0 er tanken og det skal BARE gå stærkt... vi må se 
 * Semaforer for hele systemet skal også kunne igangsættes af ADC_vect så den kan tikke derudaf selv tråårr jeg O_o
 * 
 * 
 * Med en ticktime på 202 us, bruges der 2 us på ticks med 1 task og stiger med est 1,3 us pr tick.
*/


class Job{
    public:
    Job* next = NULL; //linked lists bør være hurtigere her O_o
    void (*func)(void);
    //alt opgøres i ticks.. ahah
    uint16_t D_r; //relative deadline
    uint8_t ID;
    Job(void (*function)()); //constructors
    Job();
    void append(Job *input);
};


//starter AVRTIC
void AVRTIC_start();



/*Queuer job med en deadline hvor den bare SKAL være afviklet...
* @param Input job der bliver queued for at blive kørt
* @param D_r den relative deadline for jobbet
*/
void QueueJob(Job* input, uint16_t D_r);

//Queuer job med en deadline men til interrupts så den ikke fucker global interrupts op
void QueueJob_INT(Job* input, uint16_t D_r);

/*
*Sætter funktionen i kø hver given periode i ticks.
*@param input Job der skal sættes i queue
*@param D_r Den relative deadline i forhold til starttidspunktet
*@param T Perioden for hvornår funktionen skal gentages 
*/
void QueueTimedJob(Job* input, uint16_t D_r, uint16_t T);