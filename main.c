#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h> 
#include<stdbool.h>
#define NUM_STUDENTS 6


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


int student_count=0, barrel=10;
PCB student[NUM_STUDENTS];  
PCB bartender;


void get_serving(PCB* s);
void drink_and_think(PCB* s);
void refill_barrel();
void print_student(PCB* s);


void print_state(PCB* s);

void* studentThread(void* arg) {
    long tid;
    tid = (long)arg;
    student[tid].pid = 0;
    student[tid].tid = tid;
    student[tid].lb_required = 3;
    student[tid].lb_consumed = 0;
    student[tid].p_type = Student;
    student[tid].t_state = waiting;

    while (true) {     
        print_state(&student[tid]);   
        pthread_mutex_lock(&mutex);
        if (barrel == 0) {
            printf("thread %ld woke up the bartender\n", tid);
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&condition_wait);
        } else {
            get_serving(&student[tid]);
        }
        pthread_mutex_unlock(&mutex);
        drink_and_think(&student[tid]);

        if (student[tid].lb_consumed >= student[tid].lb_required)
        {
            student[tid].t_state = terminated;
            printf("thread %ld terminated\n", tid);
            break;
        }
        student[tid].t_state = waiting;
        print_state(&student[tid]);
    }
}


void* bartenderThread(void* arg) {
    bartender.pid = 0;
    bartender.wake_count = 3;
    bartender.t_state = waiting;
    bartender.wake_count = 3;

    printf("Bartender created\n");
    while (true) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition_wait, &mutex);
        refill_barrel();
        pthread_mutex_unlock(&mutex);

        if (bartender.wake_count == 0)
        {
            printf("Bartender has died");
            bartender.t_state = terminated;
            break;
        }
        
        bartender.wake_count--;
    }
}


void main () {

    // create the bartender thread
    pthread_t Bartender;
    int bthread = pthread_create( &Bartender, NULL, bartenderThread, NULL );    
    if (bthread)
    {
        printf("error creating the thread");
    }   
    
    int thread_created;
    pthread_t thread[NUM_STUDENTS];
    // create 3 student threads
    for (long i = 0; i < NUM_STUDENTS; i++)
    {
        thread_created = pthread_create( &thread[i], NULL, studentThread, (void*) i);
        if (thread_created)
        {
            printf("error creating the thread");
        }
    }

    sleep(300);
}



void get_serving(PCB* s){
    s->t_state = refilling;
    print_state(s);
    barrel--;
    printf("barrel: %d\n", barrel);
    sleep(2);
}


void refill_barrel(){
    bartender.t_state = refilling;
    printf("Bartender is doing a refill\n");
    // make this a random value
    barrel = 10;
    sleep(6);
}

void drink_and_think(PCB* s){
    s->t_state = drinking;
    print_state(s);
    sleep(2);
    s->lb_consumed++;

    // thinking
    s->t_state = thinking;
    print_state(s);
    sleep(2);
}

void print_state(PCB* s) {
    switch (s->t_state)
    {
    case 1:
        printf("Thread %d is refilling, Type: %d \n", s->tid, s->p_type);
        break;
    case 2:
        printf("Thread %d is drinking, Type: %d \n", s->tid, s->p_type);
        break;
    case 3:
        printf("Thread %d is thinking, Type: %d \n", s->tid, s->p_type);
        break;
    case 4:
        printf("Thread %d is waiting, Type: %d \n", s->tid, s->p_type);
        break;
    default:
        printf("Thread %d is terminated, Type: %d \n", s->tid, s->p_type);
        break;
    }
}
