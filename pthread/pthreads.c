#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#define LEFT_ENDPOINT 5
#define RIGHT_ENDPOINT 100
#define NUM_TRAPEZOIDS 10000000
#define NUM_THREADS 16

// Global Mutex
pthread_mutex_t mutex_sum;
double integralsum;

/* Computes Value of Function to be Integrated */
__attribute__((const)) float func(float x)
{
	return (x+1)/sqrt(x*x + x + 1);
}

typedef struct thread_data{

	float a;
	float b;
	int n;
	float(*f)(float);

} thread_data;


double compute_gold(float, float, int, float (*)(float));
double compute_using_pthreads(float, float, int, float (*)(float));

int main()
{
	int n = NUM_TRAPEZOIDS;
	float a = LEFT_ENDPOINT;
	float b = RIGHT_ENDPOINT;

	struct timeval tstart, tstop;
	double elapsed;


	gettimeofday(&tstart, NULL);
	double reference = compute_gold(a, b, n, func);
	gettimeofday(&tstop, NULL);
	


	elapsed = (tstop.tv_sec  + 1E-6 * tstop.tv_usec) - ( tstart.tv_sec + 1E-6 * tstart.tv_usec);
	
	printf("Reference Solution Computed On The CPU = %f \n", reference);
	printf("Reference Solution Computed In: %f ms\n", (double)1000*elapsed);
	
	gettimeofday(&tstart, NULL);
	double pthreads_result = compute_using_pthreads(a , b, n, func);
	gettimeofday(&tstop, NULL);

	elapsed = (tstop.tv_sec  + 1E-6 * tstop.tv_usec) - ( tstart.tv_sec + 1E-6 * tstart.tv_usec);
//	elapsed = (tstart.tv_sec - tstop.tv_sec) + (tstart.tv_nsec - tstop.tv_nsec);

	printf("Solution Computed Using PThreads = %f \n", pthreads_result);
	printf("Solution Computed Using %d Threads In: %f ms\n", NUM_THREADS, (double)1000*elapsed);
	
}

double compute_gold(float a, float b, int n, float(*f)(float))
{
	float h = (b-a)/(float)n;	// Height of trapezoid

	double integral = (f(a) + f(b))/2.0;
	
	for(int k = 1; k <= n-1; k++)
	{
		integral += f(a+k*h);
//		printf("Integral_r: %f\n", integral);
	}
	integral = integral*h;

	return integral;
}


void *compute_thread(void *argument){

	// Unpack Input Data
	thread_data *t_data = (thread_data *)argument;
	float a = t_data->a;	
	float b = t_data->b;
	int n = t_data->n;

	double integral;

	// Create Height
	float h = (b-a)/(float)n;

	// Calculate Integral
	integral = (func(a) + func(b))/2.0;
	for(int k = 1; k <= n-1; k++){
		integral += func(a+k*h);
//		printf("Integral: %f\n", integral);
	}

	// Multiply by height
	integral = integral * h;

	// Mutex Sum Protection
	pthread_mutex_lock(&mutex_sum);
	integralsum += integral;
	pthread_mutex_unlock(&mutex_sum);

	
	return(NULL);
}


double compute_using_pthreads(float a, float b, int n, float(*f)(float))
{
	// PThread Types
	pthread_t threads[NUM_THREADS];
	pthread_mutex_init(&mutex_sum, NULL);
	
	// Thread Iterator 
	long t_num;
	// PThread Return Code
	int rc;
	// Void Pointer
	void *status = 0;
	// Local Variable
	double local_sum;


	integralsum = 0.0;
	// Create Buckets

	float endpoint_bucket = (b-a)/NUM_THREADS;
	int trap_bucket = n/NUM_THREADS;

	// Create thread to compute each bucket
	for(t_num = 0; t_num < NUM_THREADS; t_num++){
		
		// Generate New Variable
		thread_data *t_data = malloc(sizeof(thread_data));
	
		// Generate Endpoints
		t_data->a = a + t_num*endpoint_bucket;
		t_data->b = a + (t_num+1)*endpoint_bucket;
		t_data->n = trap_bucket;
		t_data->f = f;
		
		// Create Threads
		rc = pthread_create(&threads[t_num], NULL, compute_thread, (void *)t_data);
		if(rc){
			printf("I'll bash ye skully");
			exit(-1);
		}
	}
	
	//Join all threads
	for(t_num = 0; t_num < NUM_THREADS; t_num++){
		rc = pthread_join(threads[t_num], &status);
		if(rc){
			printf("You fook'm wut mate");
			exit(-1);
		}
	}
	
	// Destroy Mutex
	pthread_mutex_destroy(&mutex_sum);
	// Pull Global Sum
	local_sum = integralsum;
	// Return
	return local_sum;
	
}
