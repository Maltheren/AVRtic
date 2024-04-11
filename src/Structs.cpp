#include<AVRTIC.h>
int JobIndex = 0;

Job::Job(void (*function)(), const int stakSize){
    ID = JobIndex;
    JobIndex++;
    func = function;
    stak = new char[stakSize]; //åhhh kriminelt... men det kun i konstructoren. vi allokerer stak i den størrelse vi nu skal bruge 
    stakPointer = (uint16_t)&stak[stakSize]; //vi gemmer hvor vores stack den må starte i memory. fordi stackpointeren tæller ned fra det her tal.
}


Job::Job(){
    ID = JobIndex;
    JobIndex++;
}
void Job::append(Job* input){
    input->next = this->next; //sætter alt i køen efter ham 
    this->next = input; //maser ham lige ind
}