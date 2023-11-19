#include "simulate.h"

sem_t analogSemm1;//, analogSem2, analogSem3, analogSem4;

void sim(){

/******************************************************************************
 * Initialisation des sémaphores
 *****************************************************************************/
	sem_t simulator_sync, ctrlNavigation_sync, ctrlCamera_sync, cameraRout;


	sem_init(&simulator_sync, 0, 0);
	sem_init(&simulator1_sync, 0, 0);
	sem_init(&simulator2_sync, 0, 0);
	sem_init(&simulator3_sync, 0, 0);
	sem_init(&ctrlNavigation_sync, 0, 0);
	sem_init(&ctrlCamera_sync, 0, 0);
	sem_init(&genAleatoire_sync, 0, 1);
    sem_init(&ctrlDest_sync, 0, 0);
    sem_init(&alarmeLow_sync, 0, 0);
    sem_init(&alarmeHigh_sync, 0, 0);
    sem_init(&cameraRout, 0, 0);


    // Structure pour les priorités
    struct sched_param param;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    if(pthread_mutex_init(&sync_connectionA2D, NULL) != 0){
		perror("pthread_mutex_init");
	}
    if(pthread_mutex_init(&sync_connectionD2A, NULL) != 0){
		perror("pthread_mutex_init");
	}

    
    struct timespec tp;
    if(0 != clock_gettime(CLOCK_REALTIME, &tp)) {
		// Print error 
		printf("Could not get start time: %d\n", errno);
		//return EXIT_FAILURE;
	}

/******************************************************************************
 * Create the different tasks
 *****************************************************************************/
    pthread_t task_pulse_handler;
    pthread_t realSpeedTask;
	pthread_t realOrientationPositionTask;
	pthread_t batterieTask;
	pthread_t cameraTask;
    pthread_t genAleatoireTask;
    pthread_t ctrlDestinationTask;
    pthread_t ctrlNavigationTask;
    pthread_t ctrlCameraTask;
    pthread_t alarmBattery10Task;
    pthread_t alarmBattery80Task;


/******************************************************************************
 * Create timers arguments
 *****************************************************************************/
    struct sigevent   realSpeedTask_event;
    struct itimerspec realSpeedTask_itime;
    timer_t           realSpeedtask_timer;

    struct sigevent   ctrlNavigation_event;
    struct itimerspec ctrlNavigation_itime;
    timer_t           ctrlNavigation_timer;

    struct sigevent   ctrlCamera_event;
    struct itimerspec ctrlCamera_itime;
    timer_t           ctrlCamera_timer;

    struct sigevent   CameraRoutine_event;
    struct itimerspec CameraRoutine_itime;
    timer_t           CameraRoutine_timer;
/******************************************************************************
 * Initialize the pulse thread tasks arguments
 *****************************************************************************/
	vitesseThread_arg simulator_args;
    vitesseThread_arg camera_args;
	controlleurThread_arg ctrlNavigation_args;
    controlleurThread_arg ctrlCamera_args;

	Voiture voiture;
    PathMap pathMap;
	simulator_args.smartCar  = &voiture;
    simulator_args.smartCar->pathMap = &pathMap;
	simulator_args.id        = 0;
	simulator_args.semaphore = &simulator_sync;
	simulator_args.starttime = tp.tv_sec;
	simulator_args.chid      = ChannelCreate(0);
	if(-1 == simulator_args.chid) {
			// Print error
			printf("Could not create channel: %d\n", errno);
	}

    camera_args.smartCar  = &voiture;
    camera_args.smartCar->pathMap = &pathMap;
	camera_args.id        = 3;
	camera_args.semaphore = &cameraRout;
	camera_args.starttime = tp.tv_sec;
	camera_args.chid      = ChannelCreate(0);
	if(-1 == camera_args.chid) {
			// Print error
			printf("Could not create channel: %d\n", errno);
	}

	Controleur ctrl;

	ctrl.taskData.pathMap = &pathMap;

    ctrlNavigation_args.control  = &ctrl;
    ctrlNavigation_args.id        = 1;
    ctrlNavigation_args.semaphore = &ctrlNavigation_sync;
    ctrlNavigation_args.starttime = tp.tv_sec;
    ctrlNavigation_args.chid      = ChannelCreate(1);
    if(-1 == ctrlNavigation_args.chid) {
            // Print error 
            printf("Could not create channel: %d\n", errno);
    }

    ctrlCamera_args.control   = &ctrl;
    ctrlCamera_args.id        = 2;
    ctrlCamera_args.semaphore = &ctrlCamera_sync;
    ctrlCamera_args.starttime = tp.tv_sec;
    ctrlCamera_args.chid      = ChannelCreate(0);
    if(-1 == ctrlCamera_args.chid) {
            // Print error
            printf("Could not create channel: %d\n", errno);

    }

    initSignal();
/******************************************************************************
 * Create the different pulse handlers
 *****************************************************************************/
    if(0 != pthread_create(&task_pulse_handler, NULL, pulse_handler, &simulator_args)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}
    if(0 != pthread_create(&task_pulse_handler, NULL, pulse_handler, &ctrlNavigation_args)) {
    // Print error
    printf("Could not create thread: %d\n", errno);
    }
    if(0 != pthread_create(&task_pulse_handler, NULL,
                        pulse_handler, &ctrlCamera_args)) {
    // Print error
    printf("Could not create thread: %d\n", errno);

    }
    if(0 != pthread_create(&task_pulse_handler, NULL,
                        pulse_handler, &camera_args)) {
    // Print error
    printf("Could not create thread: %d\n", errno);

    }
    
/***************************************************************
 *   instantiation de taches ou thread  de la partie continue du systeme
 **************************************************************/
    if(0 != pthread_create(&realSpeedTask, NULL, vitesseRoutine, &simulator_args)) {
        // Print error 
        printf("Could not create thread: %d\n", errno);
    }
    if(0 != pthread_create(&realOrientationPositionTask, NULL, orientationPositionRoutine, &simulator_args)) {
        // Print error 
        printf("Could not create thread: %d\n", errno);
    }
    if(0 != pthread_create(&batterieTask, NULL, batterieRoutine, &simulator_args)) {
        // Print error 
        printf("Could not create thread: %d\n", errno);
    }
    /*if(0 != pthread_create(&cameraTask, NULL, cameraRoutine, &simulator_args)) {
        // Print error
        printf("Could not create thread: %d\n", errno);

    }*/
    if(0 != pthread_create(&cameraTask, NULL, cameraRoutine, &camera_args)) {
        // Print error
        printf("Could not create thread: %d\n", errno);
    }

/**************************************************************************
 *   instantiation des taches ou thread  de la partie numerique du systeme
 **************************************************************************/
    param.sched_priority = 5;
	pthread_attr_setschedparam(&attr, &param);
	if(0 != pthread_create(&genAleatoireTask, &attr, generateurAleatoireRoutine, &ctrl)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}

	param.sched_priority = 4;
	pthread_attr_setschedparam(&attr, &param);
	if(0 != pthread_create(&ctrlDestinationTask, &attr, ctrlDestinationRoutine, &ctrl)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}
	param.sched_priority = 3;
	pthread_attr_setschedparam(&attr, &param);
	if(0 != pthread_create(&ctrlNavigationTask, &attr, ctrlNavigationRoutine, &ctrlNavigation_args)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}
    param.sched_priority = 2;
    pthread_attr_setschedparam(&attr, &param);
    if(0 != pthread_create(&ctrlCameraTask, &attr, ctrlCameraRoutine, &ctrlCamera_args)) {
        // Print error
        printf("Could not create thread: %d\n", errno);

    }
    param.sched_priority = 6;
	pthread_attr_setschedparam(&attr, &param);
	if(0 != pthread_create(&alarmBattery10Task, &attr, alarmBattery10, &ctrl)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}
    param.sched_priority = 6;
	pthread_attr_setschedparam(&attr, &param);
	if(0 != pthread_create(&alarmBattery80Task, &attr, alarmBattery80, &ctrl)) {
		// Print error
		printf("Could not create thread: %d\n", errno);
	}

/******************************************************************************
 * Create the different timers
 *****************************************************************************/
    if(0 != init_timer(&realSpeedTask_event, &realSpeedTask_itime, &realSpeedtask_timer,
                        simulator_args.chid, 100)) {
        // Print error 
        printf("Could not create timer: %d\n", errno);
    }
    if(0 != init_timer(&ctrlNavigation_event, &ctrlNavigation_itime, &ctrlNavigation_timer,
						ctrlNavigation_args.chid, 100)) {
		// Print error
		printf("Could not create timer: %d\n", errno);
	}
    if(0 != init_timer(&ctrlCamera_event, &ctrlCamera_itime, &ctrlCamera_timer,
                        ctrlCamera_args.chid, 50)) {
        // Print error
        printf("Could not create timer: %d\n", errno);

    }
    if(0 != init_timer(&CameraRoutine_event, &CameraRoutine_itime, &CameraRoutine_timer,
                        camera_args.chid, 45)) {
        // Print error
        printf("Could not create timer: %d\n", errno);

    }

/***********************************************************************
 *          joindre les thread de la partie continue du system 
 **********************************************************************/
    // Wait for the threads to finish 
    if(0 != pthread_join(realSpeedTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    if(0 != pthread_join(task_pulse_handler, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
        //return EXIT_FAILURE;
    }
    if(0 != pthread_join(realOrientationPositionTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    if(0 != pthread_join(batterieTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    if(0 != pthread_join(cameraTask, NULL)) {
        // Print error
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    
/***********************************************************************
 *          joindre les thread de la partie numerique du system 
 **********************************************************************/

    if(0 != pthread_join(genAleatoireTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    if(0 != pthread_join(ctrlDestinationTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
   if(0 != pthread_join(ctrlNavigationTask, NULL)) {
        // Print error 
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }

    if(0 != pthread_join(ctrlCameraTask, NULL)) {
        // Print error
        printf("Could not wait for thread: %d\n", errno);
       //return EXIT_FAILURE;
    }
    if(0 != pthread_join(alarmBattery10Task, NULL)) {
       // Print error
       printf("Could not wait for thread: %d\n", errno);
      //return EXIT_FAILURE;
   }
   if(0 != pthread_join(alarmBattery80Task, NULL)) {
       // Print error
       printf("Could not wait for thread: %d\n", errno);
      //return EXIT_FAILURE;
   }

}
