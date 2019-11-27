/*
Operating Systems: Thread synchronizations in c programming language
Names: Britney Beckford, Racquel Bailey, Shannon Henry, Rumone Anderson
*/


#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<string.h>
#include<unistd.h> 
#include<stdbool.h>
#define NUM_STUDENTS 15


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

volatile int running_threads = 0;
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;

// This controls access to refills
sem_t barrel_access;
// signal goes off when bartender needs to be called
sem_t barrel_refill;

// this is global because the students need to know the current state of the bartender
// initialization of bartender structure
PCB bartender;

// size of the barrel to be modified by one thread at a time
// set to 10 for debugging
volatile int barrel=10;


void get_serving(PCB* s, bool* served);
void drink_and_think(PCB* s, bool* served);
void refill_barrel();
void print_student(PCB* s);

// utility functions
void pcb_initialize(PCB *pcb);

// control flow functions
void* student_function(void* arg);
void* bartender_function(void* arg);


void print_thread_values(PCB pcb[], int size) {
    printf("BARREL: %d\n", barrel);
    printf("Number of students: %d\n", size);
    printf("+-------+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
    printf("| pid\t| tid\t| refilling\t| drinking\t| thinking\t| waiting\t| terminated\t| p_type\t| required\t| consumed\t| TT\t\t|\n");
    printf("+-------+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");
    for (int i = 0; i < size; i++)
    {
        printf("| %d\t| %d\t| %d\t\t| %d\t\t| %d\t\t| %d\t\t| %d\t\t| %s\t| %d\t\t| %d\t\t| %d\t\t|\n", pcb[i].pid, pcb[i].tid, (pcb[i].t_state == 1)?1:0, (pcb[i].t_state == 2)?1:0, (pcb[i].t_state == 3)?1:0, (pcb[i].t_state == 4)?1:0, (pcb[i].t_state == 5)?1:0, (pcb[i].p_type == Student)?"Student": "Bartender", pcb[i].lb_required, pcb[i].lb_consumed, pcb[i].TT);
    }
    printf("+-------+-------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+\n");    
}

int main () {
    // Student data structure to store all student threads
    PCB students[NUM_STUDENTS];

    // initialize semaphore
    sem_init(&barrel_access, 0, 1);
    sem_init(&barrel_refill, 0, 0);

    // create the bartender thread
    pthread_t Bartender;   

    int num_students = 0;
    int time_to_display = 0;
    // accept user input number student threads to use
    printf("\nSTUDENT BARTENDER THREAD SYNCHRONIZATION SIMMULATION >----------------------------\n");
    printf("\nEnter the amount of students and the program will do the rest [maximum of 15 students]: ");
    scanf("%d", &num_students);
    printf("\nWould you like to modify the time taken to display the current state of the threads [To use default of 2 enter <0>]: ");
    scanf("%d", &time_to_display);

    if (num_students == 0) {
        printf("The simmulation is complete no student was entered");
        return 0;
    }

    if (time_to_display == 0){
        time_to_display = 2;
    }

    printf("\n-> Simmulation Started\n");

    bartender.pid = -1;
    // returns 0 if the thread was successfully created
    int bthread = pthread_create( &Bartender, NULL, bartender_function, &bartender );    
    if (bthread)
    {
        printf("Error creating BARTENDER thread");
    }   
    

    sleep(4);
    pthread_t thread[num_students];
    // create 3 student threads
    for (int i = 0; i < num_students; i++) {
        // sets the process id to the current index
        students[i].pid = i;
        students[i].tid = i + 1;
        int sthread = pthread_create( &thread[i], NULL, student_function, &students[i]);
        if (sthread) {
            printf("Error creating STUDENT thread");
        }
    }


    // status display
    // interrupt summutlation and display pcb state currnetly
    while (true) {
        print_thread_values(students, num_students);
        printf("\nNumber of student threads running: %d\n", running_threads);

        sleep(time_to_display);

        if (barrel == 0 && bartender.wake_count == 0 || running_threads == 0)
        {
            return 0;
        }
        
    }   
    sem_destroy(&barrel_access);
}


void* student_function(void* arg) {
    // sData is for student data
    time_t start;
    time(&start);
    // register as a running thread
    pthread_mutex_lock(&running_mutex);
    running_threads++;
    pthread_mutex_unlock(&running_mutex);

    PCB *sData = (PCB*) arg;
    bool served = false;   

    sData->p_type = Student;
    pcb_initialize(sData); 

    while (sData->lb_consumed != sData->lb_required) {
        sem_wait(&barrel_access);
        // critical section.
        if (barrel == 0){
            if(bartender.t_state == waiting) {
                sem_post(&barrel_refill);
                sleep(1);
            } else if (bartender.t_state == refilling) {
                sleep(1);
            } else if (bartender.t_state == terminated) {
                sem_post(&barrel_access);
                break;                
            }
        } else {
            get_serving(sData, &served);
            sleep(1);
        }
    
        sem_post(&barrel_access);
        drink_and_think(sData, &served);
        sData->t_state = waiting;
    }

    time_t stop;
    time(&stop);

    sData->TT = (int)stop - (int)start;
    printf("Student %d was terminated\n", sData->pid);
    sData->t_state = terminated;

    pthread_mutex_lock(&running_mutex);
    running_threads--;
    pthread_mutex_unlock(&running_mutex);

    sleep(3);
}

void* bartender_function(void* arg) {
    time_t start;
    time(&start);
    // bData is for bartender data
    PCB *bData = (PCB*) arg;

    bData->tid = pthread_self();
    bData->p_type = Bartender;
    pcb_initialize(bData);

    // printf("bartender started\n");
    while (bData->wake_count > 0) {
        printf("Bartender is sleeping\n");
        sem_wait(&barrel_refill);
        printf("\nBartender was woken up\n");
        printf("Bartender needs to sleep %d times before death\n", bData->wake_count);
        refill_barrel(bData);
        printf("BARREL FILLED\n");
    }   

    time_t stop;
    time(&stop);

    bData->TT = (int)stop - (int)start;

    printf("The bartender died of crippling depression after serving college kids all night\n");
    printf("Bartender thread terminated\n");
    bData->t_state = terminated;
}


void pcb_initialize(PCB *pcb) {
    pcb->t_state = waiting;
    pcb->TT = 0;
    if (pcb->p_type == Bartender) {
        pcb->lb_required = -1;
        pcb->lb_consumed = -1;
        pcb->wake_count = 3;
    } else {
        pcb->wake_count = -1;
        pcb->lb_required = rand() % 5 + 1;
        pcb->lb_consumed = 0;
    }
}

void get_serving(PCB* s, bool* served){
    s->t_state = refilling;
    sleep(2);
    barrel--;
    *served = true;
}

void refill_barrel(PCB* b){
    b->t_state = refilling;
    printf("Bartender is doing a refill\n");
    sleep(2);
    barrel = rand() % 50 + 1;
    b->wake_count--;
    b->t_state = waiting;
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





