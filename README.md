## An RTIC inspired Kernal, for AVR/Arduino

Very work in porgresss.... more to come hopefully

## What this
It sets up an executionqueue and uses timer2 to make it EDF scheduled. This allows the user to queue jobs in different segments of program. It was created since there was a need to be able to run a semaphore driven task system along with a freerunning ADC on the arduino (audio streaming!?). The needs were a lesser tick size than 1 ms, and no need for interrupting a task unless another task reeally has a need to.

## How
Create a job class with a linked function to be executed. 




## Why 



