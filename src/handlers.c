#include "myconst.h"
#include "base.h"
#include "uMPStypes.h"
#include "handlers.h"
#include "utils.h"
#include "types11.h"
#include "scheduler.h"
#include "syscall.h"

/* Variabili esterne (kernel e scheduler) */
extern state_t* pnew_old_areas[NUM_CPU][NUM_AREAS];
extern state_t pstate[NUM_CPU];
extern pcb_t *currentProc[NUM_CPU]; /* puntatore al processo in esecuzione attuale */

/* Handler per le System Call e i Breakpoint */
/* Invocate da SYSCALL(number, arg1, arg2, arg3); */
void sysbp_handler()
{	 
	/* recupero il numero della CPU attuale */
	U32 prid = getPRID();
	/* recupero il processo chiamante */
	state_t *OLDAREA = pnew_old_areas[prid][OLD_SYSBP];
	/* incremento il PC del processo chiamante, per evitare loop */
	/* in questo caso non serve aggiornare anche t9 */
	/* (pag 28, 3.7.2 Student Guide) */
	OLDAREA->pc_epc += WORD_SIZE; /* 4 */
	/* recupero i parametri della SYSCALL dalla OLDAREA */
	U32 *num_syscall =  &(OLDAREA->reg_a0);
	U32 *arg1 		 =  &(OLDAREA->reg_a1);
	U32 *arg2		 =  &(OLDAREA->reg_a2);
	U32 *arg3		 =  &(OLDAREA->reg_a3);

	/* recupero lo stato (kernel-mode o user-mode) */
	U32 *old_status = &(OLDAREA->status);
	
	/* recupero la causa (tipo di eccezione sollevato) */
	U32 *old_cause = &(OLDAREA->cause);
	
	/* carico il processo corrente */
	pcb_t *processoCorrente = getCurrentProc(prid);

	/* se il processo era in kernel mode */
	if( (*old_status & STATUS_KUc) == 0 )
	{
		/* controllo se il processo non ha un handler custom */
		if(processoCorrente->custom_handlers[NEW_SYSBP] == NULL)
		{			
			/* eseguo la SYSCALL adeguata */
			switch(*num_syscall)
			{
				case CREATEPROCESS:
					/* int create_process(state_t *statep, int priority) */
					create_process((state_t*) *arg1, (int) *arg2);
					break;
				case CREATEBROTHER:
					/* int create_brother(state_t *statep, int priority) */
					create_brother((state_t*) *arg1, (int) *arg2);
					break;
				case TERMINATEPROCESS:
					/* void terminate_process() */
					terminate_process();
					break;
				case VERHOGEN:
					/* void verhogen(int semKey) */
					verhogen((int) *arg1);
					break;
				case PASSEREN:
					/* void passeren(int semKey) */
					passeren((int) *arg1);
					break;
				case GETCPUTIME:
					/* int get_cpu_time()  */
					get_cpu_time();
					break;
				case WAITCLOCK:
					/* void wait_clock() */
					wait_clock();
					break;
				case WAITIO:
					/* int wait_io(int intNo, int dnum, int waitForTermRead) */
					wait_io((int) *arg1, (int) *arg2, (int) *arg3);
					break;
				case SPECPRGVEC:
					/* void specify_prg_state_vector(state_t *oldp, state_t *newp) */
					specify_prg_state_vector((state_t*) *arg1, (state_t*) *arg2);
					break;
				case SPECTLBVEC:
					/* void specify_tlb_state_vector(state_t *oldp, state_t *newp) */
					specify_tlb_state_vector((state_t*) *arg1, (state_t*) *arg2);
					break;
				case SPECSYSVEC:
					/* void specify_sys_state_vector(state_t *oldp, state_t *newp) */
					specify_sys_state_vector((state_t*) *arg1, (state_t*) *arg2);
					break;
				/* Se e' stata chiamata una SYSCALL non esistente */
				default:
					/* killo il processo */
					terminate_process();
					break;
			} /*switch*/
			/* ritorno il controllo al processo chiamante */
			LDST(pnew_old_areas[prid][OLD_SYSBP]);
		} /* if */
		/* se il processo ha un custom handler lo chiamo */
		else
		{
			/* copio il processo chiamante nella OLD Area custom */
			copyState(OLDAREA, processoCorrente->custom_handlers[OLD_SYSBP]); 
			/* chiamo l'handler custom */
			LDST(processoCorrente->custom_handlers[NEW_SYSBP]);
		}
	}
	/* se e' stata chiamata la SYSTEM CALL in User Mode lancio TRAP */
	else
	{
		/* copiare SYSCALL OLD AREA -> PROGRAM TRAP OLD AREA */
		copyState(pnew_old_areas[prid][OLD_SYSBP], pnew_old_areas[prid][OLD_TRAP]); 
		/* settare Cause a 10 : Reserved Instruction Exception*/
		pnew_old_areas[prid][OLD_TRAP]->cause = EXC_RESERVEDINSTR;
		/* sollevare PgmTrap, se la sbriga lui */
		trap_handler();
	}
}

void trap_handler()
{
	// TEMPORANEO
	PANIC();
	
	int cause = getCAUSE();
	/* se il processo ha dichiarato un handler per Program Trap
	 * lo eseguo, altrimenti killo il processo e tutta la progenie
	 */
	/* ottengo il processore corrente */
	U32 prid = getPRID();
	/* ottengo il processo chiamante */
	state_t *OLDAREA = pnew_old_areas[prid][OLD_TRAP];
	/* carico il processo corrente */
	pcb_t *processoCorrente = getCurrentProc(prid);
	/* controllo se il processo ha un handler custom */
	if(processoCorrente->custom_handlers[NEW_TRAP] != NULL)
	{ 
		/* copio il processo chiamante nella OLD Area custom */
		copyState(OLDAREA, processoCorrente->custom_handlers[OLD_TRAP]); 
		/* chiamo l'handler custom */
		LDST(processoCorrente->custom_handlers[NEW_TRAP]);
	}
	/* altrimenti elimino il processo e tutti i figli */
	else
	{
		terminate_process();
	}	
}

void tlb_handler()
{
	/* se il processo ha dichiarato un handler per TLB Exeption
	 * lo eseguo, altrimenti killo il processo e tutta la progenie
	 */
	/* ottengo il processore corrente */
	U32 prid = getPRID();
	/* ottengo il processo chiamante */
	state_t *OLDAREA = pnew_old_areas[prid][OLD_TLB];
	/* carico il processo corrente */
	pcb_t *processoCorrente = getCurrentProc(prid);
	/* controllo se il processo ha un handler custom */
	if(processoCorrente->custom_handlers[NEW_TLB] != NULL)
	{ 
		/* copio il processo chiamante nella OLD Area custom */
		copyState(OLDAREA, processoCorrente->custom_handlers[OLD_TLB]); 
		/* chiamo l'handler custom */
		LDST(processoCorrente->custom_handlers[NEW_TLB]);
	}
	/* altrimenti elimino il processo e tutti i figli */
	else
	{
		terminate_process();
	}	
}

void ints_handler()
{
	
	/* Determino da quale linea proviene l'interrupt 
	 * NOTA: Ogni gestione dovrebbe avvenire in mutua esclusione
	 * per evitare che due CPU, che ricevano un interrupt, inizino
	 * a risolvere lo stesso interrupt (quello con maggiore priorità)
	 * NOTA2: Sappiamo che se ci sono più interrupt pending verranno
	 * tutti risolti prima di ritornare al processo in esecuzione in 
	 * origine. Comunque conviene dare un return dopo ogni caso in modo
	 * da schedulare convenientemente la risoluzione alla CPU più idonea
	 * 
	 * NOTA2: Ricordarsi di ripristinare il processo e/o lo scheduler
	 * poiché la terminazione dell'handler non ripristina automaticamente
	 * 
	 * */
	int cause = getCAUSE();
	if (CAUSE_IP_GET(cause, INT_PLT)){
		/* Fine del TIME_SLICE */
		int id = getPRID();
		if (currentProc != NULL){
			state_t *updated = pnew_old_areas[id][OLD_INTS];
			copyState(updated, &(currentProc[id]->p_s));
		}
		/* Ricarico lo scheduler */
		if (pstate[id].pc_epc == (memaddr)scheduler)
		LDST(&pstate[id]);
	}	
	if (CAUSE_IP_GET(cause, INT_TIMER)){
		/* pseudo-clock */
		return;
	}
	if (CAUSE_IP_GET(cause, INT_DISK)){
		
		return;
	}
	if (CAUSE_IP_GET(cause, INT_TAPE)){
		
		return;
	}
	if (CAUSE_IP_GET(cause, INT_UNUSED)){
		/* Network */
		return;
	}
	if (CAUSE_IP_GET(cause, INT_PRINTER)){
		
		return;
	}
	if (CAUSE_IP_GET(cause, INT_TERMINAL)){
		/* ATTENZIONE ai subdevice! */
		return;
	}
	 	
}


/* CAUSE di Interrupts = I/O terminata da un device
	 *                       Interval Timer o Local Timer scaduto
	 * 
	 * Nel registro Cause.IP c'è la il device che ha fatto Int
	 * la priorità viene data alla linea minore,
	 * all'interno della stessa linea, al device minore,
	 * nei terminali viene data priorità alla scrittura sulla lettura
	 * 
	 * OPERAZIONI da effettuare:
	 * * Acknowledgment dell Interrupt:
	 * 	  (device) scrivere nel registro del device il comando di ACK
	 *    (timer)  settare un nuovo valore
	 * 
	 * * V sul semaforo associato al device o al timer, restituire lo stato del device al processo
	 * 
	 * * Se la V non sblocca nessun processo, salviamo lo stato del device (interrupt anticipato, ovvero SYS8 ritardata rispetto al compl. dell' 		 *   interrupt)
	 */
