#include<AVRTIC.h>
int JobIndex = 0;

Job::Job(void (*function)()){
    ID = JobIndex;
    JobIndex++;
    func = function;
    
    
}


Job::Job(){
    ID = JobIndex;
    JobIndex++;

}


void Job::append(Job* input){

    input->next = this->next; //sætter alt i køen efter ham 
    this->next = input; //maser ham lige ind
}