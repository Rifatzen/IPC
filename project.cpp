#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>


using namespace std;

#define NOT_USING 0
#define WANT_TO_USE 1
#define USING 2

#define GRP_SIZE 1
#define STUDENT_SIZE 3

int printing_states[17];

int printers[4];

sem_t binding_usable; // how many binders are free
sem_t binding_full; // how many are occupied

queue<int> binding;

sem_t students[17];

pthread_mutex_t printer_mutex;

pthread_mutex_t test_lock;
pthread_mutex_t info_lock;


class Info
{
public:
    int id;
    int p;
    int first;
};

void init_semaphore()
{
    for(int i=0;i<17;i++){
        sem_init(&students[i] ,0,0);
        printing_states[i] = NOT_USING;
        printers[i] = 0;
    }
    pthread_mutex_init(&printer_mutex, NULL);
    pthread_mutex_init(&test_lock, NULL);
    pthread_mutex_init(&info_lock, NULL);
}


void test_printer(int id)
{
    if(printing_states[id] == WANT_TO_USE && printers[id%4 +1] == 0){
        printing_states[id] = USING;
        printers[id%4 +1 ] = 1;
        sem_post(&students[id]);
    }
}

void UsePrinter(Info arg)
{

    
    
    pthread_mutex_lock(&printer_mutex);
    Info t_info = arg;

    printing_states[t_info.id] = WANT_TO_USE;

    test_printer(t_info.id);

    pthread_mutex_unlock(&printer_mutex);

    sem_wait(&students[t_info.id]);
}

void LeavePrinter(Info arg)
{
    
    pthread_mutex_lock(&printer_mutex);

    Info t_info  = arg;

    printing_states[t_info.id] = NOT_USING;
    printers[t_info.id %4 +1] = 0;

    for(int i=0;i<GRP_SIZE ; i++){
        //printf("1trying to notify %d\n", t_info.first +i );

        if(t_info.first+i != t_info.id){
            test_printer(t_info.first + i);
        }
    }
    for(int i=1;i<=STUDENT_SIZE;i++){
        if(i < t_info.first || i > (t_info.first+GRP_SIZE-1 ) ){
            //printf("2trying to notify %d\n", i );
            test_printer(i);
        }
    }

    pthread_mutex_unlock(&printer_mutex);

}


void * do_stuff(void * arg)
{

    //pthread_mutex_lock(&info_lock);
    Info t_info = *((Info*) arg);


    pthread_mutex_lock(&test_lock);
    printf("arg with %d \n", t_info.id);
    fflush(stdout);
    pthread_mutex_unlock(&test_lock);


    //pthread_mutex_unlock(&info_lock);

    //while(true){

        UsePrinter(t_info);

        pthread_mutex_lock(&test_lock);
        printf("%d is using printer %d \n" , t_info.id, t_info.p);
        fflush(stdout);
        pthread_mutex_unlock(&test_lock);



        if(t_info.id %4 == 0){
            sleep(3);
        }

        LeavePrinter(t_info);
        return NULL;
    //}
}




int main(void)
{
    // pthread_t thread1;
    // pthread_t thread2;

    // init_semaphore();

    // int p = 1;
    // int id1 = 1;
    // int id2 = 2;

    // Info t1;
    // t1.id = 1;
    // t1.p = 1;


    // Info t2;
    // t2.id = 2;
    // t2.p = 1;

    pthread_t threads[3];

    // pthread_t thread1;
    // pthread_t thread2;
    // pthread_t thread3;
    

    init_semaphore();

    for(int i=0;i<STUDENT_SIZE;i++){


        //pthread_mutex_lock(&info_lock);
        
        Info *std_info = new Info();

        std_info->id = i+1;
        std_info->p = (i+1)%4 +1;

        std_info->first = i/5 +1;

        //pthread_mutex_unlock(&info_lock);

        
        // printf("Creating %d\n" , std_info.id);
        // fflush(stdout);
        // pthread_mutex_unlock(&test_lock);
        pthread_create(threads+i, NULL, do_stuff, (void *) std_info);

        //pthread_join((threads[i]), NULL);  

    }

    // for(int i=0;i<STUDENT_SIZE;i++){
    //     pthread_join(threads[i], NULL);
    // }

    //pthread_create(&thread1 , NULL,UsePrinter, (void *) &t1 );
    //pthread_create(&thread2 , NULL,UsePrinter, (void *) &t2 );


    while(1);
    return 0;
    
}


