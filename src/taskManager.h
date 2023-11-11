#ifndef TASKMANAGER_
#define TASKMANAGER_
#include <stdint.h>       /* int32_t uint32_t */
#include <stdlib.h>       /* EXIT_SUCCESS */
#include <pthread.h>      /* pthread_t pthread_create pthread_join */
#include <semaphore.h>    /* sem_t sem_init sem_wait sem_post */
#include <time.h>
#include <signal.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "voiture.h"
#include "controleur.h"

#define TASK_PULSE_CODE _PULSE_CODE_MINAVAIL

typedef union pulse_msg{
	struct _pulse pulse;
} pulse_msg_t;

struct vitesseThread_arg {
	sem_t*   semaphore; /* Synchronization semaphore pointer */
	Voiture*   smartCar;
	uint32_t id;        /* Task id */
	uint32_t starttime; /* Global start time */
	int32_t  chid;      /* Task channel id */
};

struct controlleurThread_arg {
	sem_t*   semaphore; /* Synchronization semaphore pointer */
	Controleur*   control;
	uint32_t id;        /* Task id */
	uint32_t starttime; /* Global start time */
	int32_t  chid;      /* Task channel id */
};

/*struct orientationPositionThread_arg {
	sem_t*   semaphore; 
	Voiture*   smartCar;
	uint32_t id;       
	uint32_t starttime; 
	int32_t  chid;      
};*/


// Semaphore pour synchroniser les taches de la partie continue
extern sem_t simulator1_sync, simulator2_sync;

// Semaphore du controleur pour les tâches non-périodique
extern sem_t genAleatoire_sync, ctrlDest_sync, alarmeLow_sync, alarmeHigh_sync;

// signal or connection
// input et ouput variable pour passe le result de partie continue 
// a la partie numerique ou controleur
struct analogToDigital{
	double vitesseReel;
	double orientationReel;
	double posXReel;
	double posYReel;
	double niveauBatterie;
	bool analyseTerminee;
	bool alarmeBatterie10;
	bool alarmeBatterie80;
};

extern analogToDigital A2D;


// input et ouput variable pour passe le result de partie numerique 
// a la partie continue
struct digitalToAnalog{
	bool chargerBatterie;
	double demarrerCycleAnalyse;
	double consigneOrientation;
	double consigneVitesse;
};
extern digitalToAnalog D2A;
// init input output connection
void initSignal();

extern pthread_mutex_t sync_connectionA2D, sync_connectionD2A;

int32_t init_timer(struct sigevent* event, struct itimerspec* itime,
		           timer_t* timer, const int32_t chanel_id,
				   const uint32_t period);

void* vitesseRoutine(void* args);
void* pulse_handler(void* args);
void* orientationPositionRoutine(void* args);
void* batterieRoutine(void* args);

void* generateurAleatoireRoutine(void *args);
void* ctrlDestinationRoutine(void *args);
void* ctrlNavigationRoutine(void *args);
void* ctrlCamaraRoutine(void *args);
void* alarmBattery10(void *args);
void* alarmBattery80(void *args);



#endif


