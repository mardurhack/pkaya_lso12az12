#include "myconst.h"
#include "listx.h"
#include "types11.h"
#include "pcb.e"
#include "asl.e"

/* Globali - comuni a tutte le CPU */
int procs; /* contatore dei processi (per tutte le CPU) */
int softprocs; /* contatore dei processi bloccati su I/O */
state_t pstate[NUM_CPU]; /* stati di load/store per le varie cpu */
extern new_old_areas[NUM_CPU][NUM_AREAS];

/* Specifiche per ogni CPU */
pcb_t *currentproc[NUM_CPU]; /* puntatore al processo in esecuzione attuale */
struct list_head readyQueue[NUM_CPU]; /* coda dei processi in stato ready */

/* TODO: allocare un processo nella readyQueue ed eseguirlo */

/* Questa funzione inserisce nella readyQueue il pcb_t passato */
void addReady(pcb_t *proc){
	int id;
	for (id=1; id<NUM_CPU; id++)
		INIT_LIST_HEAD(&(readyQueue[id]));
	insertProcQ(&(readyQueue[1]), proc);
}

/* Questa funzione si occupa di estrare un processo dalla readyQueue
 * e di caricarne lo stato sulla prima CPU "eligible" che trova 
 * Una CPU è "eligible" quando: ??? */
void loadReady(){
	pcb_t *torun = removeProcQ(&(readyQueue[1]));
	INITCPU(1, &(torun->p_s), &(new_old_areas[1]));
}

/* AVVIO DELLO SCHEDULER - Passaggio del controllo */
void scheduler(){
	loadReady();	
}