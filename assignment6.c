#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>

#define numPhilo 5
pthread_mutex_t chopstick[numPhilo];

// Gaussian function from Mccamish                                  
int randomGaussian(int mean, int stddev) {
    double mu = 0.5 + (double) mean;
    double sigma = fabs((double) stddev);
    double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
    double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;
    if (rand() & (1 << 5)) 
        return (int) floor(mu + sigma * cos(f2) * f1);
    else            
        return (int) floor(mu + sigma * sin(f2) * f1);
}

// Dining philosopher actions
void *actions(void *n) {
    int x = *((int*)n);

    //Deadlock solution
    int left, right;
    if(x < numPhilo-1) { 
        left = x;
        right = x+1; 
    }
    else { 
        left = 0;
        right = numPhilo-1;
    }

    //Seeds rng for rand
    srand(x);

    int total_eating = 0;
    int total_thinking = 0;
    int eat_time;
    int think_time;
    int cycles = 0;

    //Stop when philosopher eats for 100 seconds
    while(total_eating < 100) { 
        cycles = cycles + 1;

        //Philosopher thinking
        think_time = randomGaussian(11, 7);
        if(think_time < 0) { think_time = 0; }

        //Simulate time spent thinking
        printf("Philosopher %d is thinking for %d seconds (total = %d)\n", x, think_time, total_thinking);
        sleep(think_time);
        total_thinking += think_time;

        //Pick up chopsticks
        int leftStick = pthread_mutex_lock(&chopstick[left]);
        int rightStick = pthread_mutex_lock(&chopstick[right]);

        if(leftStick != 0 || rightStick != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
        }

        //Philosopher eating
        eat_time = randomGaussian(9, 3); 
        if(eat_time < 0) { eat_time = 0; }

        //Simulate time spent eating
        printf("Philosopher %d is eating for %d seconds (total = %d)\n", x, eat_time, total_eating);
        sleep(eat_time); 
        total_eating += eat_time;

        //Put down chopsticks
        leftStick = pthread_mutex_unlock(&chopstick[left]);
        rightStick = pthread_mutex_unlock(&chopstick[right]);

        if(leftStick != 0 || rightStick != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
    } 
    //Print results for each philosopher
    printf("Philosopher %d done with meal\n", x);
    printf("Philosopher %d thought for %d seconds, ate for %d seconds over %d cycles.\n", 
            x, total_thinking, total_eating, cycles);     

    // Free philo num
    free(n); 
} 

int main(int argc, char *argv[]){
    int init, create, join, destroy;
    int *num; 
    pthread_t philosopher[numPhilo];
    
    // Initialize mutex
    for(int n = 0; n < numPhilo; n++) {
        init = pthread_mutex_init(&chopstick[n], NULL);
        if(init != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
    }
    // Create threads (also assigns phil num n, to different address)
    for(int n = 0; n < numPhilo; n++) {
        num = malloc(sizeof(*num)); 
        *num = n;
        create = pthread_create(&philosopher[n], NULL, actions, num);
           if(create != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
           }
    }
    // Wait for threads to terminate
    for(int n = 0; n < numPhilo; n++) {
        join = pthread_join(philosopher[n], NULL);
        if(join != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
    }
    // Destroy mutex, aka "cleanup"
    for(int n = 0; n < numPhilo; n++) {
        destroy = pthread_mutex_destroy(&chopstick[n]);
        if(destroy != 0) {
            printf("%s\n", strerror(errno));
            exit(1);
        }
    }
    return 0;
}
