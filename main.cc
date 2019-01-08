******************************************************************
* The Main program with the two functions. A simple
* example of creating and using a thread is provided.
******************************************************************/

#include "helper.h"
#include <iostream>
#include <pthread.h>
#include <string>


void *producer (void *parameter);
void *consumer (void *queue_ptr);

struct Parameters{
  int sem_id;
  int size_of_queue;
  int producer_id;
  int consumer_id;
  int number_of_jobs;
  int *queue;

  int prod_c;
  int cons_c;
};


int main (int argc, char** argv)
{
  if(argc != 5){
    cerr << "You have to provide the size of the queue, the number of jobs to generate for each producer, ";
    cerr << "the number of producers, and the number of consumers" << endl;
    return 1;
  }

  int size_of_queue = check_arg(*(argv+1));
  int number_of_jobs = check_arg(*(argv+2));
  int number_of_producers = check_arg(*(argv+3));
  int number_of_consumers = check_arg(*(argv+4));

  if(number_of_producers + number_of_consumers >= 2000){
    cerr << "The system cannot create the amount of threads required according to your input parameters." << endl;
    return 1;
  }

  // Creating and initialising Semaphores
  int sem = sem_create(SEM_KEY, 4);
  sem_init(sem, 0, 1); //ERROR HANDLING
  sem_init(sem, 1, size_of_queue);
  sem_init(sem, 2, 0);
  sem_init(sem, 3, 0);


  // Creating Producers
  pthread_t producerid[number_of_producers];

  Parameters parameter;
  parameter.sem_id = sem;
  parameter.size_of_queue = size_of_queue;
  parameter.number_of_jobs = number_of_jobs;
  parameter.queue = new int[size_of_queue];
  parameter.prod_c = 0;
  parameter.cons_c = 0;

  for(int i = 0; i < number_of_producers; i++){
    parameter.producer_id = i + 1;
    pthread_create(&producerid[i], NULL, producer, (void *) &parameter);
    sem_wait(sem, 3); // waiting for the id to be initialised.
  }


  // Creating Consumers
  pthread_t consumerid[number_of_consumers];

  for(int i = 0; i < number_of_consumers; i++){
    parameter.consumer_id = i + 1;
    pthread_create(&consumerid[i], NULL, consumer, (void *) &parameter);
    sem_wait(sem, 3); // waiting for the id to be initialised.
  }


  // Joining all threads
  for(int i = 0; i < number_of_consumers; i++){
    pthread_join(consumerid[i], NULL);
  }

  for(int i = 0; i < number_of_producers; i++){
    pthread_join(producerid[i], NULL);
  }

  // Closing Semaphores and ending program
  sem_close(sem);
  return 0;
}

void *producer (void *parameter)
{
  // Initialise Parameters
  Parameters* p = (Parameters *) parameter;

  int producer_id = p->producer_id;
  sem_signal(p->sem_id, 3);

  int count = p->number_of_jobs;

  while(count > 0){
    //Produce new item
    int sleep_rand = rand() % 5 + 1;
    int duration_rand = rand() % 10 + 1;

    sleep(sleep_rand);
    if(sem_timedwait(p->sem_id, 1)) break;
    sem_wait(p->sem_id, 0);
    //Critical Section start
    cout << "Producer(" << producer_id << "): ";
    cout << "Job id " << p->prod_c;
    cout << " duration " << duration_rand << endl;
    p->queue[p->prod_c] = duration_rand;
    p->prod_c = (p->prod_c + 1) % p->size_of_queue;
    //Critical Section end
    sem_signal(p->sem_id, 0);
    sem_signal(p->sem_id, 2);
    count--;
  }
  printf("Producer(%d): No more jobs to generate.\n", producer_id);

  pthread_exit(0);
}

void *consumer (void *parameter)
{
  Parameters* p = (Parameters *) parameter;
  int consumer_id = p->consumer_id;
  sem_signal(p->sem_id, 3);
  int job;


  while(1){
    //Timed Semaphore
    if(sem_timedwait(p->sem_id, 2)) break;
    sem_wait(p->sem_id, 0);
    //Critical section start
    job = p->queue[p->cons_c];
    cout << "Consumer(" << consumer_id << "): ";
    cout << "Job id " << p->cons_c;
    cout << " executing sleep duration " << job << endl;
    p->cons_c = (p->cons_c + 1 ) % p->size_of_queue;
    //Critical section end
    sem_signal(p->sem_id, 0);
    sem_signal(p->sem_id, 1);
    sleep(job); //Sleeps for the duration specified in job
    printf("Consumer(%d): Job id %d completed\n", consumer_id, p->cons_c);
  }
  printf("Consumer(%d): No more jobs left.\n", consumer_id);

  pthread_exit (0);

}
