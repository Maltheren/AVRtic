#include <Arduino.h>
#include <AVRTIC.h>
#include <WreckRegs.h>



/**
 * @brief Mest for at undgå rod og globale variabler, indeholder alle brugsvariabler der deles mellem structs og funktioner i AVRtic
 * 
 */
namespace executionQueue{
    uint16_t timeSinceShift = 0;
    Job* Head;
    uint16_t idleStackPointer;
    bool jobShiftFlag = 0;
    Job* CurrentExe;
}








/**
* @brief Sætter timeren der skal køre alt det her op
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

/**
* Kører interrupt og gemmer alt CPU'en var igang
* 
*/
void __attribute__((naked, noinline)) interruptJob(){
    //Der skulle gerne kun være 2 ting tilbage i stack lige nu, som er retur addressen for en kaldefunktion og returaddresen for denne funtion.
    PUSHREGS(); //kopierer alle værdier til stacken.
    executionQueue::CurrentExe->stakPointer = SP; //kopirer hvor hans stackpointer var
    SP = (uint16_t)executionQueue::Head->stakPointer; //hapser vores nye jobs stackpointer ind i stedet
    //? Han tager simpelthen bare og accepterer vi er i en breakout i stedet for at prøve at skjule det.... makes sense
    POPREGS();
    RET();
}


//idle job til når cpu'en idler
void idleFunction();
Job idle(idleFunction, 100);

/*************************************
 * Funktion til når systemet er idle
 * ***********************************
*/
void idleFunction(){
    idle.D_r = 0x0FFF;
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
    if(current == &idle){
        if(current->next == &idle){
            current->append(input);
            return;
        }
    }
    if(current->D_r > input->D_r){
        //Ohshit vi er nødt til at skifte job
        input->next = executionQueue::Head;
        executionQueue::Head = input;
        Serial.println("CChange");
        interruptJob();
        return;
    }
    
    while((current->next)->D_r <= input->D_r){ //!runway
        current = current->next; //hopper til den næste
    }
    current->append(input); //plopper ham ind når han møder en der har en større deadline end ham selv
}

void QueueJob(Job *input, uint16_t D_r){
    Serial.print("Queuing: ");
    Serial.println(SP, HEX);
    delay(10);
    QueueJob_INT(input, D_r);
}


void DumpQueue(){
    Job* current = executionQueue::Head;
    Serial.print("IDLEPOINTER: ");
    Serial.println((uint16_t)&idle, HEX);
    Serial.println("QUEUE:");
    int i = 0;
    while(current->next != &idle){
        Serial.print(i);
        Serial.print(": \t");
        Serial.println((uint16_t)&current, HEX);
        i++;
        current = current->next;
    }
}

void AVRTIC_prepare(){
    idle.D_r = 0x0000;
    executionQueue::Head = &idle;
    idle.next = &idle;
}

void AVRTIC_start(){
    cli();
    SetupPriorityUpdater(3, 100);


    sei();
    Serial.println("setup the queue going into loop");
    DumpQueue();
    while (true)
    {   
        cli();
        if(executionQueue::Head->next == &idle){ //backstopper
            idle.next = &idle; 
        }
        sei();
        executionQueue::CurrentExe = executionQueue::Head; //sætter den aktuelle 
        ExcecuteContained(executionQueue::CurrentExe); //funktionen med sin egen stack
        executionQueue::Head = executionQueue::Head->next; //tar næste
        
    }

}




void QueueTimedJob(Job* input, uint16_t D_r, uint16_t T){



}




