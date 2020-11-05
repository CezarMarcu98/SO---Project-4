#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "so_scheduler.h"
#include "queueAndList.h"


typedef struct Thread {
	tid_t id;
	int priority;
	int time;
	sem_t semaphore;
	int blockFlag;
} Thread;

typedef struct Waiting {
	TLista waitingThr;
	int ioIndex;
	int devWoke;
} waitPoll;

typedef struct {
	so_handler *handler;
	unsigned int priority;
	pthread_barrier_t barr;
} Argumente;

typedef struct Scheduler {
	unsigned int cuantaT;
	unsigned int nrDev;
	priorityQueue *ThrReady;
	waitPoll *waiting;
	priorityQueue *newThreads;
	Thread *running;
} Scheduler;

Scheduler *scheduler;


void create_First_Thr(void)
{
	int ok;

	scheduler->running = calloc(1, sizeof(Thread));
	if (scheduler->running == NULL)
		exit(1);
	scheduler->running->priority = -1;
	scheduler->running->id = pthread_self();
	scheduler->running->time = scheduler->cuantaT;

	/* Sincronizez threadul principal */
	ok = sem_init(&scheduler->running->semaphore, 0, 0);
	if (ok != 0)
		exit(1);

}

int so_init(unsigned int thrTime, unsigned int nr)
{
	int i;

	if (nr > 256 || thrTime < 1 || scheduler != NULL)
		return -1;

	scheduler = calloc(1, sizeof(Scheduler));
	if (scheduler == NULL)
		exit(1);

	scheduler->nrDev = nr;
	scheduler->cuantaT = thrTime;

	scheduler->waiting = calloc(nr, sizeof(waitPoll));
	if (scheduler->waiting == NULL)
		exit(1);

	for (i = 0; i < nr; ++i) {
		scheduler->waiting[i].waitingThr =
		(TLista)calloc(1, sizeof(TCelula));

		scheduler->waiting->ioIndex = i;
		scheduler->waiting->devWoke = 0;
	}

	scheduler->ThrReady = calloc(1, sizeof(priorityQueue));
	if (scheduler->ThrReady == NULL)
		exit(1);

	scheduler->newThreads = calloc(1, sizeof(priorityQueue));
	if (scheduler->newThreads == NULL)
		exit(1);

	create_First_Thr();

	return 0;
}


void so_end(void)
{
	int i;
	Thread *thr;

	if (scheduler != NULL) {
		while (scheduler->newThreads->head) {
			thr = headQ(scheduler->newThreads);
			popQ(scheduler->newThreads);
			pthread_join(thr->id, NULL);
			free(thr);
		}

		for (i = 0; i < scheduler->nrDev; ++i)
			DistrugeL(&scheduler->waiting[i].waitingThr);
		free(scheduler->waiting);

		while (scheduler->ThrReady->head)
			popQ(scheduler->ThrReady);
		free(scheduler->ThrReady);

		while (scheduler->newThreads->head)
			popQ(scheduler->newThreads);
		free(scheduler->newThreads);

		if (scheduler->running != NULL)
			free(scheduler->running);

		free(scheduler);
		scheduler = NULL;
	}
}

void take_thread(void)
{
	/* threadul curent si-a terminat executia sau */
	/* nu este nici un alt thread ce ruleaza acum */
	int ok;
	Thread *next = headQ(scheduler->ThrReady);

	if (next == NULL) {
		scheduler->running = NULL;
		return;
	}
	/* distrug semaforul threadului curent */
	ok = sem_destroy(&scheduler->running->semaphore);
	if (ok != 0)
		exit(1);
	/* il bag in coada de threaduri disponibile pt join */
	insertQ(scheduler->newThreads, (void *)scheduler->running, 0);
	popQ(scheduler->ThrReady);
	scheduler->running = next;

	/* ii permit noului thread sa ruleze */
	ok = sem_post(&scheduler->running->semaphore);
	if (ok != 0)
		exit(1);
}

int check_preempt(int *block)
{
	Thread *next = headQ(scheduler->ThrReady);

	if (next == NULL)
		return -1;
	if (scheduler->running->time == 0) {
		/* verific expirarea cuantei */
		scheduler->running->time = scheduler->cuantaT;
		/* daca alt thread are o prioritate mai mare sau */
		/* egala, atunci trebuie inlocuit threadul curent */
		if (next->priority >= scheduler->running->priority)
			return 1;
	}
	/* verific aparitia in coada ready a unui thread */
	/* mai important decat cel curent */
	if (next->priority > scheduler->running->priority)
		return 1;
	if (scheduler->running->blockFlag) {
	/* operatie blocanta -> threadul ce ruleaza */
	/* trebuie sa astepte pana termina I/O-ul actiunea */
		(*block)++;
		return 1;
	}
	return 0;
}

void so_exec(void)
{
	int ok;
	int block = 0;

	if (scheduler->running == NULL)
		take_thread();
	scheduler->running->time--;

	if (check_preempt(&block) == 1) {
		Thread *thr = headQ(scheduler->ThrReady);
		Thread *preemted_thread = scheduler->running;

		popQ(scheduler->ThrReady);
		/* daca threadul nu este blocant si nu isi */
		/* finalizeaza task-ul, atunci se intoarce */
		/* in coada ready */
		if (block == 0) {
			insertQ(scheduler->ThrReady,
			(void *)scheduler->running,
			scheduler->running->priority);
		}
		scheduler->running = thr;
		/* permit noului thread sa ruleze */
		ok = sem_post(&scheduler->running->semaphore);
		if (ok != 0)
			exit(0);
		/* opresc din rulare threadul inlocuit */
		ok = sem_wait(&preemted_thread->semaphore);
		if (ok != 0)
			exit(0);

	} else if (check_preempt(&block) == -1)
		return;
}

void *thread_go(void *args)
{
	Argumente *args2 = args;
	Thread *thr = calloc(1, sizeof(Thread));

	if (thr == NULL)
		return NULL;

	thr->priority = args2->priority;
	thr->time = scheduler->cuantaT;
	thr->id = pthread_self();
	int ok;
	/* sincronizez threadurile */
	ok = sem_init(&thr->semaphore, 0, 0);
	if (ok != 0)
		exit(1);

	/* inserez threadul curent in coada ready pentru a putea fi programat */
	insertQ(scheduler->ThrReady, (void *)thr, thr->priority);
	/* odata inserat threadul copil in coada ready anunt parintele*/
	/* ca isi poate continua executia */
	pthread_barrier_wait(&args2->barr);
	pthread_barrier_destroy(&args2->barr);

	/* astept ca planificatorul sa execute acest thread */
	ok = sem_wait(&thr->semaphore);
	if (ok != 0)
		exit(1);

	args2->handler(thr->priority);

	free(args);
	take_thread();
	return NULL;
}

tid_t so_fork(so_handler *function, unsigned int priority)
{
	if (function == NULL || priority > 5)
		return 0;

	pthread_t new_thread;

	Argumente *args = (Argumente *)calloc(1, sizeof(Argumente));

	args->priority = priority;
	args->handler = function;
	/* bariera pt initializarea threadului copil */
	pthread_barrier_init(&args->barr, NULL, 2);

	pthread_create(&new_thread, NULL, &thread_go, (void *)args);
	/* astept threadul copil sa fie instantiat */
	/* inainte de a continua rularea threadului parinte */
	pthread_barrier_wait(&args->barr);

	/* verific finalizarea threadului parinte */
	if (scheduler->running->id == pthread_self())
		so_exec();
	return (tid_t)new_thread;
}


int so_wait(unsigned int id_dev)
{

	if (id_dev >= scheduler->nrDev)
		return -1;

	/* inserez in lista threadurile de asteptare pentru acest dispozitiv */
	Insert(&scheduler->waiting[id_dev].waitingThr,
	(void *)scheduler->running, sizeof(Thread *));

	scheduler->waiting->devWoke++;
	scheduler->running->blockFlag = 1;
	so_exec();

	return 0;
}

int so_signal(unsigned int id)
{
	int count = 0;
	Thread *thr;
	/* pointer catre lista de threaduri in asteptare */
	TLista *aux = &scheduler->waiting[id].waitingThr;

	if (id >= scheduler->nrDev)
		return -1;

	/*scot cate element cu element din lista de threaduri*/
	/*in asteptare si le pune inapoi in coada ready pentru a*/
	/*putea fi planificate*/
	while ((thr = firsList(*aux)) != NULL) {
		popList(aux);
		thr->blockFlag = 0;

		insertQ(scheduler->ThrReady, (void *)thr,
			    thr->priority);
		count++;
	}
	so_exec();

	return count;
}
