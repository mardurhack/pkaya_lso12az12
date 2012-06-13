/* Constants */
/* TODO: trovare  modo di leggere num CPU da emulatore */
#define NUM_CPU 4
#define STATUS_TE 0x08000000
#define RESET 0

/* NUMERI DELLE SYSTEM CALL */

#define CREATEPROCESS		1
#define CREATEBROTHER		2
#define TERMINATEPROCESS	3
#define VERHOGEN			4
#define PASSEREN			5
#define GETCPUTIME			6
#define WAITCLOCK			7
#define WAITIO				8
#define SPECPRGVEC			9
#define SPECTLBVEC			10
#define SPECSYSVEC			11

/* New e Old Area */
#define NUM_AREAS 8
#define NEW_SYSBP 0
#define NEW_TRAP 1
#define NEW_TLB 2
#define NEW_INTS 3
#define OLD_SYSBP 4
#define OLD_TRAP 5
#define OLD_TLB 6
#define OLD_INTS 7

