
typedef struct psetrec {
	int  member; /* integer for determining membership */
	int  path ;  /* data */
	struct psetrec *next ;
} PSETBOX, *PSETPTR ; /* path set record */

/* cell types */
#define STDCELLTYPE       1
#define EXCEPTTYPE        2
#define PADTYPE           3
#define PORTTYPE          4
#define EXTRATYPE         5
#define PADGROUPTYPE      6
#define HARDCELLTYPE      7
#define SOFTCELLTYPE      8
/* pin types */
#define PINTYPE           1
#define PASS_THRU         2
#define SWAP_PASS         3
