#include <avr/interrupt.h>
#include <util/atomic.h>

#include "AmOS_Config.h"



typedef enum { READY,RUNNING,WAITING,BLOCKED,FINISHED} taskState; 


typedef struct _tcb{
	void* stack;
	taskState state;
	void (*callBack)(void);
    int8_t priority;
	int8_t id;
	_tcb* next;
}taskInstance;

extern taskInstance *currentTask,*idleTask,*headList,*holder;


void AmOs_Switch();
#include "hardware.h"
static taskInstance *currentTask,*idleTask,*headList,*holder;
static void idleTaskfunc(void);
static void setIdleHook(void (*callBack)(void));
taskInstance* createTask(void (*callBack)(void),uint16_t stackSize,int8_t priority,int8_t id);
void init_AmOS();
void execution();
void run_AmOS();

////////////////////////////////////////////////

static void idleTaskfunc(void)
{
    while (1) { }
}

static void setIdleHook(void (*callBack)(void)){
	idleTask->callBack=callBack;	
}

void execution(){
	 currentTask->callBack();
	 ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
	 currentTask->state=FINISHED;
     AmOs_Switch();	
		}		
		
}

void init_AmOS(){
headList=NULL;
holder=NULL;
idleTask=NULL;
currentTask=NULL;
idleTask=createTask(idleTaskfunc,0,DEFAULT_MED_PRIORITY,00);	
}

void run_AmOS(){
	timers();
	AmOs_Switch();
}


taskInstance* createTask(void (*callBack)(void),uint16_t stackSize,int8_t priority,int8_t id){
	taskInstance* instance;
	instance=(taskInstance*) malloc(sizeof(taskInstance));
	uint16_t stSize=DEFAULT_STACK_SIZE;
	if(stackSize!=0){
    stSize=stackSize;
	}
    instance->stack=(void*)allocateStack(stSize,execution);
	instance->state=READY;
	instance->callBack=callBack;
    instance->priority=priority;
	instance->next=(taskInstance*)NULL;
	instance->id=id;
	
	
	if(headList==NULL){headList=instance;headList->next=NULL;idleTask=instance;}
	else{
		holder=headList;
		while(1){
			if(holder->next==NULL){
				break;
			}
			holder=holder->next;
		}
		holder->next=instance;
		holder=NULL;
	}
	
	
	return instance;
}

void AmOs_Switch(){
	     ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
			if(currentTask==NULL){
				currentTask=idleTask;
			}
	      holder=currentTask->next;
	      while(1)
		  {
	     	if(holder==NULL){holder=headList;}
		    
			else if(holder->state!=FINISHED){
		    Cont_Switch(holder);
		    break;}
			
			
		    holder=holder->next;
		  }		
		}
}

///// just for debugging in arduino
void printing(){
	holder=headList;
	while(1){
		if(holder==NULL){
			holder=headList;
		}
		Serial.println(holder->id);
		holder=holder->next;
		
	}
	
}
