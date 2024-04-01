#include <Arduino.h>
#include <AVRTIC.h>
#include <WreckRegs.h>

namespace executionQueue{ //den nøjervigtige kø
    uint16_t timeSinceShift = 0;
    Job* Head;
    uint16_t idleStackPointer;
    bool jobShiftFlag = 0;
    Job* CurrentExe;
}








/*
* @param Prescaler som siger hvor tit tælleren må inkrementeres, 
*   0 = stoppet, 1 = 1, 2 = 8, 3 = 64, 4 = 256, 5 = 1024,  
*
* @param Værdi der trigger comparematch. eksempelvis hvis prescaler er sat så Timer2 stiger hver 16 clockcyklus så 
*/
void SetupPriorityUpdater(uint8_t prescaler, uint8_t compareMatch){
    //sætter timere op så de
    TCCR2A = B0000010; //gør så timer0 ikke er bundet til nogle outputs. OG at den nulstiller ved en compare match se ISR(TIMER0_OVF_vect)
    TCCR2B = prescaler & B00000111; //clearer det register vi skal bruge.
    OCR2A = compareMatch; //hvor mange ticks der går før vi skal nulstille registret og køre en interrupt
    TIMSK2 = B00000010; //gør så vi kun kører på OCR0A
    TIFR2 = B00000111; //slukker de 3 flag så vi er sikre på vi ikke bare starter en interrupt
 
}


//Tvinger processoren til at skifte til et andet job
Job* TestJob;

//sætter stackpointeren så den er self contained
void __attribute__((naked, noinline)) ExcecuteContained(Job* input){
    //en vigtig ting her er vist at vi ikke vil have noget pushet i stak når vi roder med den... så vi prøer lige det her
    PUSHREGS();
    cli(); //skifter til containeren for vores job kan man sige..
    executionQueue::idleStackPointer = SP;
    SP = input->stakPointer; //banker en ny stakpointer ind...
    sei();
    input->func();
    cli();//gør vi er sikre på ikke at blive afbrudt mens vi skifter stack
    SP = executionQueue::idleStackPointer;
    sei();
    POPREGS();
    RET(); //kører return
}


/// @brief Skal kaldes i slutningen af en interrupt. fordi vi skal vide hvor mange ting der er tilbage i stacken.
/// @param input Job der skal interruptes med.
void __attribute__((naked, noinline)) interruptJob(){
    //Der skulle gerne kun være 2 ting tilbage i stack lige nu, som er retur addressen for en kaldefunktion og returaddresen for denne funtion.


    PUSHREGS(); //kopierer alle værdier til stacken.
    executionQueue::CurrentExe->stakPointer = SP; //kopirer hvor hans stackpointer var
    SP = (uint16_t)executionQueue::Head->stakPointer; //hapser vores nye jobs stackpointer ind i stedet
    
    //@TODO FIXXXXXXXXXXX


    
    uint16_t voresReturn = *(uint16_t*)SP; //NEJ FUCKFUCKFUCKFUCKFUCKFUCK
    RET();
}

void QueueChange(Job* input){
    //forbereder køen
    input->next = executionQueue::Head;
    executionQueue::Head = input;
    executionQueue::jobShiftFlag = 1;
}


//idle job til når cpu'en idler
void idleFunction();
Job idle(idleFunction, 100);

/*************************************
 * Funktion til når systemet er idle
 * ***********************************
*/
void idleFunction(){

}


__attribute__((weak)) void breakout(){
    if(executionQueue::Head == &idle){
        debugPCHIGH(0);
    }
    else{
        debugPCLOW(0);
    }
}




ISR(TIMER2_COMPA_vect){ //Opdaterer køøøen
    debugPCHIGH(0);
    Job *current = executionQueue::Head;
    while(current != &idle){
        current->D_r -= 1;
        current = current->next;
    }
    debugPCLOW(0);
}



void QueueJob_INT(Job *input, uint16_t D_r){
    input->D_r = D_r;
    Job *current = executionQueue::Head;
    while((current->next)->D_r <= input->D_r){
        current = current->next; //hopper til den næste
    }
    current->append(input);
}

void QueueJob(Job *input, uint16_t D_r){
    cli();
    QueueJob_INT(input, D_r);
    sei();
}

void AVRTIC_start(){
    cli();
    
    SetupPriorityUpdater(3, 100);
    idle.D_r = 0xFFFF;
    executionQueue::Head = &idle;
    idle.next = &idle;
    sei();

    while (true)
    {   
        cli();
        if(executionQueue::Head->next == &idle){
            idle.next = &idle; 
        }
        sei();
        //breakout();
        executionQueue::Head->func(); //kører funktionen
        executionQueue::Head = executionQueue::Head->next; //tar næste
        
    }

}




void QueueTimedJob(Job* input, uint16_t D_r, uint16_t T){



}




