#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h> 
#include<stdbool.h>
#define NUM_STUDENTS 3


enum thread_state {
    refilling=1,
    drinking,
    thinking,
    waiting,
    terminated
};

enum process_type {
    Student = 0,
    Bartender  
};


// process control block struct
struct pcb {
    int pid;
    int tid;
    enum thread_state t_state;
    enum process_type p_type;
    int lb_required;
    int lb_consumed;
    int wake_count;
    int TT;
}typedef PCB;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_wait = PTHREAD_COND_INITIALIZER;


int student_count=0, barrel=5;

void get_serving(PCB* s, bool* served);
void drink_and_think(PCB* s, bool* served);
void refill_barrel();
void print_student(PCB* s);

// utility functions
void pcb_initialize(PCB *pcb);
int print_thread_values(PCB *pcb);

// control flow functions
void* student_function(void* arg);
void* bartender_function(void* arg);


void main () {
    // Student data structure to store all student threads
    PCB students[NUM_STUDENTS];  
    // initialization of bartender structure
    PCB bartender;

    // create the bartender thread
    pthread_t Bartender;
    // returns 0 if the thread was successfully created
    bartender.pid = -1;
    int bthread = pthread_create( &Bartender, NULL, bartender_function, &bartender );    
    if (bthread)
    {
        printf("Error creating BARTENDER thread");
    }   
    
    // TODO number of thread should be entered by the user

    pthread_t thread[NUM_STUDENTS];
    // create 3 student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        // sets the process id to the current index
        students[i].pid = i;
        int sthread = pthread_create( &thread[i], NULL, student_function, &students[i]);
        if (sthread) {
            printf("Error creating STUDENT thread");
        }
    }

    // interrupt summutlation and display pcb state currnetly
    while (true) {
        // TODO use system interrup to pause and resume processing
        // TODO improve print function
        sleep(1);
        printf("BARREL: %d\n", barrel);
        printf("+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
        printf("| pid\t| tid\t\t| refilling\t| drinking\t| thinking\t| waiting\t| terminated\t| p_type\t| required\t| consumed\t| wake_count\t| TT\t\t|\n");
        printf("+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
        print_thread_values(&bartender);
        printf("+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
        for (int i = 0; i < (int)(sizeof(students)/sizeof(PCB)); i++)
        {
            print_thread_values(&students[i]);
        }
        printf("+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
    }   
}


void* student_function(void* arg) {
    // sData is for student data
    PCB *sData = (PCB*) arg;
    bool served = false;

    sData->tid = pthread_self();
    sData->p_type = Student;
    pcb_initialize(sData); 

    while (true) {     
        pthread_mutex_lock(&mutex);
        if (barrel == 0) {
            printf("Student %d wakes up the bartender \n", sData->pid);
            pthread_cond_signal(&condition_wait);
        } else {
            get_serving(sData, &served);
        }
        pthread_mutex_unlock(&mutex);
        drink_and_think(sData, &served);
        if (sData->lb_consumed >= sData->lb_required)
        {
            sData->t_state = terminated;
            break;
        }
        sData->t_state = waiting;
    }
    // printf("student info: %d\n", sData->tid);    
}

void* bartender_function(void* arg) {
    // bData is for bartender data
    PCB *bData = (PCB*) arg;

    bData->tid = pthread_self();
    bData->p_type = Bartender;
    pcb_initialize(bData);

    while (true) {
        if (bData->wake_count == 0)
        {
            printf("Bartender has died\n");
            bData->t_state = terminated;
            break;
        } else {
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&condition_wait, &mutex);
            refill_barrel(bData);
            pthread_mutex_unlock(&mutex);
        }
        bData->t_state = waiting;
        bData->wake_count--;
    }
    // printf("bartender info: %d\n", bData->tid);
}


void pcb_initialize(PCB *pcb) {
    // rand() % 4 + 1
    pcb->t_state = waiting;
    pcb->TT = 0;
    if (pcb->p_type == Bartender) {
        pcb->lb_required = -1;
        pcb->lb_consumed = -1;
        pcb->wake_count = 3;
    } else {
        pcb->wake_count = -1;
        pcb->lb_required = 3;
        pcb->lb_consumed = 0;
    }
}


int print_thread_values(PCB *pcb) {
    printf("| %d\t| %d\t| %d\t\t| %d\t\t| %d\t\t| %d\t\t| %d\t\t| %s\t| %d\t\t| %d\t\t| %d\t\t| %d\t\t|\n", pcb->pid, pcb->tid, (pcb->t_state == 1)?1:0, (pcb->t_state == 2)?1:0, (pcb->t_state == 3)?1:0, (pcb->t_state == 4)?1:0, (pcb->t_state == 5)?1:0, (pcb->p_type == Student)?"Student": "Bartender", pcb->lb_required, pcb->lb_consumed, pcb->wake_count, pcb->TT);
    return 0;
}


void get_serving(PCB* s, bool* served){
    *served = true;
    s->t_state = refilling;
    barrel--;
    sleep(2);
}


void refill_barrel(PCB* b){
    b->t_state = refilling;
    printf("Bartender is doing a refill\n");
    // make this a random value
    sleep(6);
    barrel = 5;
}

void drink_and_think(PCB* s, bool* served){
    if (*served) {
        s->t_state = drinking;
        sleep(2);
        s->lb_consumed++;

        // thinking
        s->t_state = thinking;
        sleep(2);
        *served=false;
    }
}





