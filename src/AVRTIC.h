#include<Arduino.h>
#include<MemoryDumper.h>

//nogle slemme ASM hacks
#define RET()  __asm__ __volatile__ ("ret" ::)
#define RETI() __asm__ __volatile__("reti" ::)
#define POP() __asm__ __volatile__("POP" ::)
#define PUSH() __asm__ __volatile__("PUSH" ::)


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
    uint16_t stakPointer;
    char* stak;
    Job(void (*function)(), const int stakSize); //constructors
    Job();
    void append(Job *input);
};

/**
 * @brief Sætter køen op så den stopper på idle
 * 
 */
void AVRTIC_prepare();

/**
 * @brief Starter AVRTIC, betragt det som at starte et while(1) loop alt skal håndteres i tasks herfra
 * 
 */
void AVRTIC_start();



/*Queuer job med en deadline hvor den bare SKAL være afviklet...
* @param Input job der bliver queued for at blive kørt
* @param D_r den relative deadline for jobbet
*/
void QueueJob(Job* input, uint16_t D_r);

//Queuer job med en deadline men til interrupts så den ikke fucker global interrupts op
void QueueJob_INT(Job* input, uint16_t D_r);

/**
 * @brief Sætter et job i en uendelig kø, så det automatisk bliver queued for hver given periode
 * @param input Jobbet der queues
 * @param D_r Den realtive deadline
 * @param T Perioden der gives en semaphor til jobbet
 */
void QueueTimedJob(Job* input, uint16_t D_r, uint16_t T);


/**
 * @brief Testfunktion tvinger en anden funktion foran i køen
 * 
 * @param input 
 */
void QueueChange(Job* input);


//Currently functions under test
extern Job* TestJob;
/**
 * @brief Kører et job med sin egen stack
 * 
 * @param input Jobbet der skal queues
 */
void ExcecuteContained(Job* input);