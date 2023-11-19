#include <iostream>
#include <fstream>
#include "voiture.h"
#include <vector>
#include <stdio.h>        /* printf */
#include <stdlib.h>       /* EXIT_SUCCESS */
#include <stdint.h>       /* int32_t uint32_t */
#include <pthread.h>      /* pthread_t pthread_create pthread_join */
#include <semaphore.h>    /* sem_t sem_init sem_wait sem_post */
#include <errno.h>        /* errno */
#include <signal.h>       /* struct sigevent */
#include <sys/neutrino.h> /* ChannelCreate ConnectAttach MsgReceive */
#include <sys/netmgr.h>   /* ND_LOCAL_NODE */
#include <time.h>         /* struct itimerspec struct timespec
                             timer_create tier_settime clock_gettime */
#include <iostream>
#include <fstream>
#include "simulate.h"

using namespace std;
/* Pulse code definition */
#define TASK_PULSE_CODE _PULSE_CODE_MINAVAIL



int main(){

	sim();

	cout << "fin du programme" << endl; 
	return 0;
}
