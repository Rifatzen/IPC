#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>
#include <random>
#include <chrono>
#include <thread>
#include<time.h>


using namespace std;

#define NOT_USING 0
#define WANT_TO_USE 1
#define USING 2

// #define PRINTING 1
// #define BINDING 2
// #define REPORTING 3

// #define GRP_SIZE 10
// #define STUDENT_SIZE 40

int PRINTING;
int BINDING;
int REPORTING;

int GRP_SIZE;
int STUDENT_SIZE;

//int printing_states[STUDENT_SIZE+10];

int* printing_states;

int printers[5];
int xys;

int total_reports;
int readers;

// sem_t binding_usable; // how many binders are free
// sem_t binding_full; // how many are occupied

// queue<int> binding;

sem_t students[100];
//sem_t* students;

sem_t binders;
//sem_t report;

pthread_mutex_t printer_mutex;
pthread_mutex_t binder_mutex;
pthread_mutex_t report_entry_mutex;
pthread_mutex_t report;



pthread_mutex_t test_lock;
pthread_mutex_t info_lock;

//pthread_t threads[STUDENT_SIZE+10];

pthread_t* threads;

time_t start_time;


class Info
{
public:
    int id;
    int p;
    int first;
};

void init_semaphore()
{
    for(int i=0;i<STUDENT_SIZE;i++){
        sem_init(&students[i] ,0,1);
        printing_states[i] = NOT_USING;
        printers[i] = 0;
    }

    // for(int i=0;i<GRP_SIZE;i++){
    //     sem_init(&leader_binder_access[i],0,-100);
    // }

    sem_init(&binders,0,2);
    //sem_init(&report,0,1);
    
    //sem_init(&total_reports, 0 ,0);

    pthread_mutex_init(&printer_mutex, NULL);
    pthread_mutex_init(&binder_mutex, NULL);
    pthread_mutex_init(&report_entry_mutex, NULL);
    pthread_mutex_init(&report,NULL);



    pthread_mutex_init(&test_lock, NULL);
    pthread_mutex_init(&info_lock, NULL);

    time(&start_time);
   

}


int poisson_distr()
{

    int lambda = 2;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::poisson_distribution<int> poissonDist(lambda);


    return poissonDist(gen);
}

int time_passed()
{
    time_t curr_time;
    time(&curr_time);

    return difftime(curr_time, start_time);
}


void test_printer(int id)
{
    if(printing_states[id] == WANT_TO_USE && printers[id%4 +1] == 0){
        printing_states[id] = USING;
        printers[id%4 +1 ] = 1;
        sem_post(&students[id]);
        //sleep(1);
    }
}

void UsePrinter(Info arg)
{
    
    pthread_mutex_lock(&printer_mutex);

    
    Info t_info = arg;


    //time_t curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();

    int curr_time = time_passed();
    printf("Student %d has arrived at the print station at time %d\n", t_info.id, curr_time);



    printing_states[t_info.id] = WANT_TO_USE;



    
    test_printer(t_info.id);

    pthread_mutex_unlock(&printer_mutex);

    sem_wait(&students[t_info.id]);
}

void LeavePrinter(Info arg)
{
    
    pthread_mutex_lock(&printer_mutex);

    Info t_info  = arg;

    //printf("total reports: %d\n" , total_reports);

    printing_states[t_info.id] = NOT_USING;
    printers[t_info.id %4 +1] = 0;


    //time_t curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
    int curr_time = time_passed();
    printf("Student %d has finished printing at time %d\n", t_info.id,curr_time);



    //printf("total reports: %d\n" , total_reports);

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

   
    Info t_info = *((Info*) arg);


    //while(true){

        sleep(poisson_distr());

        UsePrinter(t_info);

        // pthread_mutex_lock(&test_lock);


        // printf("%d is using printer %d, %d \n" , t_info.id, t_info.p, t_info.first);
        // fflush(stdout);
        // pthread_mutex_unlock(&test_lock);


        sleep(PRINTING);

        LeavePrinter(t_info);


        


        if(t_info.id % GRP_SIZE == 0){


            for(int i=t_info.first-1 ; i<t_info.first+GRP_SIZE-1 ; i++){
                //printf("joining %d for %d\n" , i, t_info.id);

                if((i+1)%GRP_SIZE != 0){
                    pthread_join(threads[i], NULL);
                }
            }

            //time_t curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
            int curr_time = time_passed();
            printf("Group %d has finished printing at time %d\n", (t_info.first-1)/GRP_SIZE +1 ,curr_time);

            sleep(poisson_distr());

            //sem_wait(&leader_binder_access[(t_info.first-1)/GRP_SIZE]);

            sem_wait(&binders);
            pthread_mutex_lock(&binder_mutex);



            //curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
            //printf("Student %d has finished printing at time %s\n", t_info.id, ctime(&curr_time));
            
            curr_time = time_passed();
            printf("Group %d has started binding at %d \n" , (t_info.first-1)/GRP_SIZE +1 ,curr_time );

            sleep(BINDING);


            //curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
            //printf("Student %d has finished printing at time %s\n", t_info.id, ctime(&curr_time));
            curr_time = time_passed();
            printf("Group %d has finished binding at %d \n" , (t_info.first-1)/GRP_SIZE +1 ,curr_time );


            pthread_mutex_unlock(&binder_mutex);
            sem_post(&binders);

            sleep(poisson_distr());


            //sem_wait(&report);
            pthread_mutex_lock(&report);

            sleep(REPORTING);
            //curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
            
            curr_time = time_passed();
            printf("Group %d has submitted report at  %d\n" , (t_info.first-1)/GRP_SIZE +1, curr_time);


            

            total_reports = total_reports +1 ;


            //printf("total reports %d\n ", total_reports );

            //sem_post(&report);
            pthread_mutex_unlock(&report);


            

        }




    //}
}

void * read_reports( void* arg)
{

    

    int sid = *((int* )arg);


    while(true){

        sleep(poisson_distr());


        pthread_mutex_lock(&report_entry_mutex);

        readers = readers +1;
        //total_reports = total_reports +1 ;

        if(readers == 1){
            //sem_wait(&report);
            pthread_mutex_lock(&report);
        }

        pthread_mutex_unlock(&report_entry_mutex);


        //sleep(1);

        //int get_total = sem_getvalue(&total_reports, &get_total);
        

        //time_t curr_time = (std::chrono::high_resolution_clock::now() - start_time).count();
        sleep(REPORTING);
        int curr_time = time_passed();
        printf("staff %d is reading at time %d . Submissions: %d\n" ,sid,curr_time, total_reports);




        pthread_mutex_lock(&report_entry_mutex);

        readers = readers-1;
        //total_reports = total_reports +1 ;

        if(readers == 0){
            //sem_post(&report);
            pthread_mutex_unlock(&report);
        }

        
        pthread_mutex_unlock(&report_entry_mutex);

        if(total_reports == (STUDENT_SIZE/GRP_SIZE)){
            break;
        }

        //sleep(2);
    
    
    }

}




int main(void)
{



    freopen("in.txt", "r", stdin);
    freopen("out.txt","w", stdout);


    scanf("%d%d%d%d%d",&STUDENT_SIZE,&GRP_SIZE, &PRINTING, &BINDING, &REPORTING);

     printing_states = new int[100];
    // students = new sem_t[100];
     threads = new pthread_t[100];

    
    pthread_t reader_teacher;
    pthread_t reader_display;


    readers = 0;
    total_reports = 0;

    

    init_semaphore();

    for(int i=0;i<STUDENT_SIZE;i++){

        
        Info* std_info = new Info();

        std_info->id = i+1;
        std_info->p = (i+1)%4 +1;

        std_info->first = (i/GRP_SIZE)*GRP_SIZE +1;


        pthread_create(threads+i, NULL, do_stuff, (void*)std_info);

        //pthread_join((threads[i]), NULL);  

    }


    //sleep(5);





    int* id1 = new int;
    int* id2 = new int;

    *id1 = 1;
    *id2 = 2;



    pthread_create(&reader_teacher, NULL, read_reports , (void*) id1 );
    pthread_create(&reader_display, NULL, read_reports , (void*) id2 );




    for(int i=0;i<STUDENT_SIZE;i++){
        if((i+1)%GRP_SIZE == 0){
            pthread_join((threads[i]), NULL);
        }
    }

    pthread_join(reader_teacher , NULL);
    pthread_join(reader_display, NULL);



    //while(1);
    return 0;
    
}


