#include "taskManager.h"


sem_t simulator1_sync, simulator2_sync, simulator3_sync;
sem_t genAleatoire_sync, ctrlDest_sync;

pthread_mutex_t sync_connectionA2D, sync_connectionD2A;
analogToDigital A2D;
digitalToAnalog D2A;
/*******************************************************
 * Timer
 *****************************************************/
int32_t init_timer(struct sigevent* event, struct itimerspec* itime,
		           timer_t* timer, const int32_t chanel_id,
				   const uint32_t period) {
	int32_t error;
	int32_t period_s;
	int32_t period_ns;

	/* Set event as pulse and attach to channel */
	event->sigev_notify = SIGEV_PULSE;
	event->sigev_coid   = ConnectAttach(ND_LOCAL_NODE, 0, chanel_id,
									    _NTO_SIDE_CHANNEL, 0);
	/* Set basic priority and set event code */
	event->sigev_priority = 0;
	event->sigev_code     = TASK_PULSE_CODE;

	/* Create timer and associate to event */
	error = timer_create(CLOCK_MONOTONIC, event, timer);
	if(0 != error) {
		printf("Error creating timer\n");
		return error;
	}

	/* Set the itime structure */
	period_s  = period / 1000;
	period_ns = (1000000 * period) - (period_s * 1000000000);
	itime->it_value.tv_sec = period_s;
	itime->it_value.tv_nsec = period_ns;
	itime->it_interval.tv_sec = period_s;
	itime->it_interval.tv_nsec = period_ns;

	/* Set the timer period */
	return timer_settime(*timer, 0, itime, NULL);
}
/*******************************************************
 * pulse handler 
 *****************************************************/
void* pulse_handler(void* args) {

	sem_t*      sync_sem;
	int32_t     rcvid;
	pulse_msg_t msg;
	int32_t     task_chid;

	/* Get the arguments */
	sync_sem  = ((vitesseThread_arg*)args)->semaphore;
	task_chid = ((vitesseThread_arg*)args)->chid;

	while(1<2) {
		/* Get the pulse message */
		rcvid = MsgReceive(task_chid, &msg, sizeof(pulse_msg_t), NULL);
		if (0 == rcvid) {
			if (TASK_PULSE_CODE == msg.pulse.code) {
				/* Release semaphore */
				if(0 != sem_post(sync_sem)) {
					/* Print error */
					printf("Could not post semaphore: %d\n", errno);
				}
			}
			else {
				/* Print error */
				printf("Unknown message received: %d\n", rcvid);
			}
		}
		else {
			/* Print error */
			printf("Message receive failed: %d (%d)\n", rcvid, errno);
		}
	}
}
/*******************************************************
 * initialiser les connexions E/S 
 *****************************************************/
 void initSignal(){

	// from analog to digital
	A2D.vitesseReel      = 0;
	A2D.orientationReel  = 0;
	A2D.posXReel         = 0;
	A2D.posYReel         = 0;
	A2D.niveauBatterie   = 0;
	A2D.analyseTerminee  = 0;
	A2D.alarmeBatterie10 = false;
	A2D.alarmeBatterie80 = false;

	//from digital to analog
	D2A.chargerBatterie      = false;
	D2A.demarrerCycleAnalyse = 0;
	D2A.consigneOrientation  = 0;
	D2A.consigneVitesse      = 0;
 }
/**********************************************************************
 *               ROUTINE DE LA PARTIE CONTINUE
 ***********************************************************************/
/*******************************************************
 * Routine de la vitesse reel
 *****************************************************/
void* vitesseRoutine(void* args) {

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;
	Voiture* 		smartCar;

	/* Get the arguments */
	sync_sem  = ((vitesseThread_arg*)args)->semaphore;
	task_id   = ((vitesseThread_arg*)args)->id;
	starttime = ((vitesseThread_arg*)args)->starttime;
	smartCar  = ((vitesseThread_arg*)args)->smartCar;

	/* Routine loop */
	while(true) {
		/* Wait for the pulse handler to release the semaphore */
		sem_wait(sync_sem);
		//Lire les donnees en provenance du controleur
		pthread_mutex_lock(&sync_connectionD2A);
		smartCar->desiredSpeed       = D2A.consigneVitesse;
		smartCar->desiredOrientation = D2A.consigneOrientation;
		smartCar->ActiveRecharge     = D2A.chargerBatterie;
		smartCar->analyseStart		 = D2A.demarrerCycleAnalyse;
    	pthread_mutex_unlock(&sync_connectionD2A);

		smartCar->vitesse(smartCar->desiredSpeed);
		//std::cout<<"LA VITESSE désirée = "<<smartCar->desiredSpeed<<std::endl;
		sem_post(&simulator1_sync);
	}
}

/***************************************************************
 * Routine de l'orientation et de la position reel du vehicule
 **************************************************************/
 void* orientationPositionRoutine(void* args){

	Voiture* 		smartCar;
	// Get the arguments 
	smartCar  = ((vitesseThread_arg*)args)->smartCar;
	//smartCar  = ((Voiture*)args);

	///* Routine loop 
	while(true) {
		sem_wait(&simulator1_sync);
		//std::cout<<"La vitesse actuel reel : "<<smartCar->realSpeed<<std::endl;
		smartCar->positionOrientation(smartCar->realSpeed, smartCar->desiredOrientation);
		//std::cout<<"LA POSITION REEL   : X = "<<smartCar->posX<<"  Y = "<< smartCar->posY<<std::endl;
		//std::cout<<"L'ORIANTATION REEL  = "<<smartCar->realOrientation<<std::endl;
		sem_post(&simulator2_sync);
	}
}

/***************************************************************
 *         Routine de la consomation d'energie
 **************************************************************/
void* batterieRoutine(void* args){

	Voiture* 		smartCar;
	// Get the arguments 
	smartCar  = ((vitesseThread_arg*)args)->smartCar;
	//smartCar  = ((Voiture*)args);

	///* Routine loop 
	while(true) {
		sem_wait(&simulator2_sync);
		smartCar->batterie(smartCar->realSpeed);
		smartCar->alimentation();
		//std::cout<<"NIVEAU BATTERIE = "<< smartCar->batteryLevel<<std::endl;
		sem_post(&simulator3_sync);
		
	}
}
/***************************************************************
 *                   Routine Camera 
 **************************************************************/
void* cameraRoutine(void* args){

	Voiture* 		smartCar;
	// Get the arguments 
	smartCar  = ((vitesseThread_arg*)args)->smartCar;
	//smartCar  = ((Voiture*)args);
	///* Routine loop 
	while(true) {
		sem_wait(&simulator3_sync);
		smartCar->camera(smartCar->posX, smartCar->posY, smartCar->analyseStart);
		//std::cout<<"CAMERA TEST SIM"<<std::endl;

		pthread_mutex_lock(&sync_connectionA2D);
		A2D.vitesseReel      = smartCar->realSpeed;
		A2D.orientationReel  = smartCar->realOrientation;
		A2D.posXReel         = smartCar->posX;
		A2D.posYReel         = smartCar->posY;
		A2D.niveauBatterie   = smartCar->batteryLevel;
		A2D.analyseTerminee  = smartCar->analyseDone;
		A2D.alarmeBatterie10 = smartCar->alarmeBatterie10;
		A2D.alarmeBatterie80 = smartCar->alarmeBatterie80;
    	pthread_mutex_unlock(&sync_connectionA2D);
		
	}
}
/**********************************************************************
 *               ROUTINE DU CONTROLLEUR (PARTIE NUMERIQUE)
 ***********************************************************************/
/***************************************************************
 * 			Generateur de destination
 **************************************************************/
void* generateurAleatoireRoutine(void *args) {

	Controleur*    controleur;
	double posX, posY;

	/* Get the arguments */
	controleur   = ((Controleur*)args);

	/* Routine loop */
	while(true) {
		sem_wait(&genAleatoire_sync);
		pthread_mutex_lock(&sync_connectionA2D);
		posX = A2D.posXReel;
		posY = A2D.posYReel;
    	pthread_mutex_unlock(&sync_connectionA2D);
    	controleur->generateurAleatoire(posX, posY);
		sem_post(&ctrlDest_sync);
	}
}

/***************************************************************
 * 	      controleur de destination 
 **************************************************************/
void* ctrlDestinationRoutine(void *args) {

	Controleur*    controleur;
	double posX, posY;
	int i = 0;

	/* Get the arguments */
	controleur   = ((Controleur*)args);

	/* Routine loop */
	while(true) {
		sem_wait(&ctrlDest_sync);
		controleur->syncCtrlTask = nop;
		pthread_mutex_lock(&sync_connectionA2D);
		posX = A2D.posXReel;
		posY = A2D.posYReel;
    	pthread_mutex_unlock(&sync_connectionA2D);
		controleur->ctrlDestination(posX, posY);
		//post le semaphore du controleur de destination
		//sem_post(&ctrlDestSync);
		controleur->taskData.pathMap->dumpImage("./pic.bmp");

		if(controleur->syncCtrlTask == getToDestination){
			//post le semaphore du generateur de destination
			//lorsqu'on arrive a destination afin de generer une 
			//nouvelle destination
			sem_post(&genAleatoire_sync);
		}
	}
}
/***************************************************************
 * 	      controleur de Navigation
 ***************************************************************/
void* ctrlNavigationRoutine(void *args) {

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;
	Controleur*    controleur;

	/* Get the arguments */
	sync_sem     = ((controlleurThread_arg*)args)->semaphore;
	task_id      = ((controlleurThread_arg*)args)->id;
	starttime    = ((controlleurThread_arg*)args)->starttime;
	controleur   = ((controlleurThread_arg*)args)->control;

	double posX, posY;
	double realSpeed;
	double realOrientation;
	double batterie;
	//int i = 0;

	/* Routine loop */
	while(true) {
		/* Wait for the pulse handler to release the semaphore */
		if(0 == sem_wait(sync_sem)) {
			pthread_mutex_lock(&sync_connectionA2D);
			posX 	  		= A2D.posXReel;
			posY 	  		= A2D.posYReel;
		 	realSpeed 		= A2D.vitesseReel;
		 	realOrientation = A2D.orientationReel;
		 	batterie 		= A2D.niveauBatterie;
	    	pthread_mutex_unlock(&sync_connectionA2D);

			controleur->ctrlNavigation(posX, posY, realSpeed, realOrientation, batterie);


			if(controleur->syncCtrlTask == getTowayPoint){
				//post le semaphore du contoleur de destination
				std::cout<<"semaphore du controle destination Sync"<<std::endl;
				sem_post(&ctrlDest_sync);
			}
			pthread_mutex_lock(&sync_connectionD2A);
			D2A.consigneVitesse     = controleur->consigneVitesse;
			D2A.consigneOrientation = controleur->consigneOrientation;
	    	pthread_mutex_unlock(&sync_connectionD2A);
		}
	}
}

/***************************************************************
 * 	      controleur de camera
 ***************************************************************/

 void* ctrlCameraRoutine(void *args) {

	struct timespec tp;
	sem_t*          sync_sem;
	uint32_t        task_id;
	uint32_t        starttime;
	uint32_t        elapsed_time;
	Controleur*    controleur;

	/* Get the arguments */
	sync_sem     = ((controlleurThread_arg*)args)->semaphore;
	task_id      = ((controlleurThread_arg*)args)->id;
	starttime    = ((controlleurThread_arg*)args)->starttime;
	controleur   = ((controlleurThread_arg*)args)->control;

	double posX, posY;
	double analyseDone;
	double realOrientation;
	double batterie;
	//int i = 0;

	/* Routine loop */
	while(true) {
		//sem_wait(&ctrlNavSync);
		sem_wait(sync_sem);
		pthread_mutex_lock(&sync_connectionA2D);
		posX 	  		= A2D.posXReel;
		posY 	  		= A2D.posYReel;
		analyseDone		= A2D.analyseTerminee;
    	pthread_mutex_unlock(&sync_connectionA2D);

		controleur->ctrlCamera(posX, posY, analyseDone);
		//std::cout<<"CAMERA TEST CTRL"<<std::endl;
		pthread_mutex_lock(&sync_connectionD2A);
		D2A.demarrerCycleAnalyse  = controleur->demarrerCycleAnalyse; 
    	pthread_mutex_unlock(&sync_connectionD2A);
		
	}
}
