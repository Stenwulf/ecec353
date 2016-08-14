/* Reference code that searches files for a user-specified string. 
 * 
 * Author: Naga Kandasamy kandasamy
 * Date: 7/19/2015
 *
 * Compile the code as follows: gcc -o mini_grep min_grep.c queue_utils.c -std=c99 -lpthread - Wall
 *
*/

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "queue.h"
#include <pthread.h>

// Worker Data Structure //
typedef struct static_data{
   queue_element_t *file_head;
   char* search_string;
} static_data;

typedef struct dynamic_data{
   queue_t *file_queue;
   char* search_string;
} dynamic_data;

typedef struct split_queue{
   queue_element_t *new_queue;
   queue_element_t *remainder_queue;
} split_queue;

// Function Prototypes //
int serialSearch(char **);
int parallelSearchStatic(char **);
int parallelSearchDynamic(char **);
void* static_thread(void*);
void* dynamic_thread(void*);
void build_work_queue(DIR*, queue_element_t*, queue_t*, struct dirent*, struct dirent*, int);

split_queue* q_split(queue_element_t*, int);

pthread_mutex_t occurrence_mutex;
pthread_mutex_t queue_mutex;

int total_occurrences = 0;

int main(int argc, char** argv)
{
	if(argc < 5){
		printf("\n %s <search string> <path> <num threads> static (or)  %s <search string> <path> <num threads> dynamic \n", argv[0], argv[0]);
		exit(EXIT_SUCCESS);
	}

	int num_occurrences;

   struct timeval start, stop; /* Start the timer. */
   gettimeofday(&start, NULL);

   num_occurrences = serialSearch(argv);		/* Perform a serial search of the file system. */
   printf("\n The string %s was found %d times within the file system.", argv[1], num_occurrences);

   gettimeofday(&stop, NULL);
   printf("\n Serial Overall execution time = %fs.\n\n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));


   /* Perform a multi-threaded search of the file system. */
   if(strcmp(argv[4], "static") == 0){

      gettimeofday(&start, NULL);
		printf("\n Performing multi-threaded search using static load balancing.\n");
	 	num_occurrences = parallelSearchStatic(argv);
	 	printf("\n The string %s was found %d times within the file system.\n", argv[1], num_occurrences);
      gettimeofday(&stop, NULL);
      printf("\n Static Overall execution time = %fs.\n\n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
   }

  	else if(strcmp(argv[4], "dynamic") == 0){
      gettimeofday(&start, NULL);
		printf("\n Performing multi-threaded search using dynamic load balancing.\n");
	 	num_occurrences = parallelSearchDynamic(argv);
	 	printf("\n The string %s was found %d times within the file system.\n", argv[1], num_occurrences);
      gettimeofday(&stop, NULL);
      printf("\n Dynamic Overall execution time = %fs.\n\n", (float)(stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec)/(float)1000000));
	}	

  	else{
		printf("\n Unknown load balancing option provided. Defaulting to static load balancing.\n");
	 	num_occurrences = serialSearch(argv);
	 	printf("\n The string %s was found %d times within the file system.\n", argv[1], num_occurrences);
  	}

	printf("\n");
  	exit(EXIT_SUCCESS);
}

/* Serial search of the file system starting from the specified path name. */
int serialSearch(char **argv)			
{
   int num_occurrences = 0;
   queue_element_t *element, *new_element;
   struct stat file_stats;
   int status; 
 
   DIR *directory = NULL;
   struct dirent *result = NULL;
   struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent) + MAX_LENGTH);

   queue_t *queue = createQueue(); /* Create and initialize the queue data structure. */
   element = (queue_element_t *)malloc(sizeof(queue_element_t));
   
	if(element == NULL){
		perror("malloc");
		 exit(EXIT_FAILURE);
  	}

   strcpy(element->path_name, argv[2]);
   element->next = NULL;
   insertElement(queue, element); 							/* Insert the initial path name into the queue. */

   while(queue->head != NULL){									/* While there is work in the queue, process it. */
	 	queue_element_t *element = removeElement(queue);
	 	status = lstat(element->path_name, &file_stats); /* Obtain information about the file. */
	 	if(status == -1){
			printf("Error obtaining stats for %s \n", element->path_name);
			free((void *)element);
			continue;
	 	}

	 	if(S_ISLNK(file_stats.st_mode)){ /* Ignore symbolic links. */
	 	} 
	 
	 	else if(S_ISDIR(file_stats.st_mode)){ /* If directory, descend in and post work to queue. */
			//printf("%s is a directory. \n", element->path_name);
			directory = opendir(element->path_name);
			if(directory == NULL){
				printf("Unable to open directory %s \n", element->path_name);
				continue;
			}

			while(1){
				status = readdir_r(directory, entry, &result); /* Read directory entry. */
				if(status != 0){
				 	printf("Unable to read directory %s \n", element->path_name);
					break;
				 }
				
			  	if(result == NULL)				/* End of directory. */
				 	break; 										  											  
				
				 if(strcmp(entry->d_name, ".") == 0)	/* Ignore the "." and ".." entries. */
					continue;
				
				 if(strcmp(entry->d_name, "..") == 0)
					continue;

				 /* Insert this directory entry in the queue. */
				 new_element = (queue_element_t *)malloc(sizeof(queue_element_t));
				 if(new_element == NULL){
				 	printf("Error allocating memory. Exiting. \n");
					exit(-1);
				 }
				
	  			 /* Construct the full path name for the directory item stored in entry. */
			 	 strcpy(new_element->path_name, element->path_name);
				 strcat(new_element->path_name, "/");
				 strcat(new_element->path_name, entry->d_name);
				 insertElement(queue, new_element);
			}
				closedir(directory);
	 } 
	 else if(S_ISREG(file_stats.st_mode)){ 		/* Directory entry is a regular file. */
				//printf("%s is a regular file. \n", element->path_name);
				FILE *file_to_search;
				char buffer[MAX_LENGTH];
				char *bufptr, *searchptr, *tokenptr;
				
				/* Search the file for the search string provided as the command-line argument. */ 
				file_to_search = fopen(element->path_name, "r");
				if(file_to_search == NULL){
					printf("Unable to open file %s \n", element->path_name);
					continue;
				} 
				else{
					while(1){
						bufptr = fgets(buffer, sizeof(buffer), file_to_search);		/* Read in a line from the file. */
						if(bufptr == NULL){
							if(feof(file_to_search)) break;
								if(ferror(file_to_search)){
									printf("Error reading file %s \n", element->path_name);
										break;
								}
						}
									 
						/* Break up line into tokens and search each token. */ 
						tokenptr = strtok(buffer, " ,.-");
						while(tokenptr != NULL){
							searchptr = strstr(tokenptr, argv[1]);
							if(searchptr != NULL){
								printf("Found string %s within file %s. \n", argv[1], element->path_name);
								num_occurrences ++;
							}
												
							tokenptr = strtok(NULL, " ,.-");						/* Get next token from the line. */
						}
					}
				}
				fclose(file_to_search);
	 }
	 else{
		printf("%s is of type other. \n", element->path_name);
	 }
	 free((void *)element);
  }

  return num_occurrences;
}


/* Parallel search with static load balancing accross threads. */
int parallelSearchStatic(char **argv)
{

   int num_occurrences = 0;
   int num_files = 0;
	queue_element_t *element, *new_element;
   struct stat file_stats;
   int status; 
 
   DIR *directory = NULL;
   struct dirent *result = NULL;
   struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent) + MAX_LENGTH);

   queue_t *dir_queue = createQueue(); /* Create and initialize the queue data structure. */
   queue_t *file_queue = createQueue();
   element = (queue_element_t *)malloc(sizeof(queue_element_t));

   // If malloc fails exit the program //
	if(element == NULL){
		perror("malloc");
		 exit(EXIT_FAILURE);
  	}

   strcpy(element->path_name, argv[2]);
   element->next = NULL;
   insertElement(dir_queue, element);

   // While there are more elements in the directory queue filter them //
   while(dir_queue->head != NULL){

      // Get File Stats from Queue Element //
	 	queue_element_t *element = removeElement(dir_queue);
	 	status = lstat(element->path_name, &file_stats);

      // If status is -1 there was an error getting file information //
	 	if(status == -1){
			printf("Error obtaining stats for %s \n", element->path_name);
			free((void *)element);
			continue;
	 	}

		// If the file is a symbolic link ignore it //
	 	if(S_ISLNK(file_stats.st_mode)){ 
	 	} 
	 
      // If the file is a directory add its contents to the queue //
	 	else if(S_ISDIR(file_stats.st_mode)){

         /* Add to queue and free previous pointer */
         build_work_queue(directory, element, dir_queue, entry, result, status);
			
         free((void *)element);
	 
		} 

      // If the file is regular then add it to the file queue //
	   else if(S_ISREG(file_stats.st_mode)){

         // Insert to Queue // 
         insertElement(file_queue, element);
			num_files++;

	   }
	   else{
		   printf("%s is of type other. \n", element->path_name);
	   }

  }
   // Create Bucket //
   int num_threads = atoi(argv[3]);
   int bucket = num_files/num_threads;
   int rc;
   int t_num;

//   printf("\n\n");
//   printQueue(file_queue);

//   printf("\n\n");
   // Assign Threads and Init Mutex //
   pthread_t worker_threads[num_threads];

   split_queue *s_queue = malloc(sizeof(split_queue));

   queue_element_t *queue_head = file_queue->head;
   size_t length = strlen(argv[1]);
   char* s_string = malloc(length+1);
   strcpy(s_string, argv[1]);
   void *t_status = 0;

   for(t_num = 0; t_num < num_threads; t_num++){
      
      s_queue = q_split(queue_head, bucket);
      
      queue_head = s_queue->remainder_queue;

      static_data *t_data = malloc(sizeof(static_data));
      t_data->file_head = s_queue->new_queue;
      t_data->search_string = s_string;


      rc = pthread_create(&worker_threads[t_num], NULL, static_thread,(void *)t_data);
      if(rc){
         printf("You dun fucked up m9");
         exit(-1);
      }
   }
   for(t_num = 0; t_num < num_threads; t_num++){
      rc = pthread_join(worker_threads[t_num], &t_status);
      if(rc){
         printf("You dun fucked up m9");
         exit(-1);
      }

   }
   pthread_mutex_destroy(&occurrence_mutex); 
   return total_occurrences;

}

split_queue* q_split(queue_element_t *queue, int bucket){

   queue_element_t *q_pointer = malloc(sizeof(queue_element_t));
   q_pointer = queue;
   split_queue *split = malloc(sizeof(split_queue));

   int increment;

   split->new_queue = queue;  	
   for(increment = 0; increment < (bucket); increment++){
      if(q_pointer->next == NULL){
         break;
      }
      else{
         q_pointer = q_pointer->next;   
         
      }
   } 

   split->remainder_queue = q_pointer->next;
   queue_element_t *terminate = malloc(sizeof(queue_element_t));
   strcpy(terminate->path_name,"EXIT");
   terminate->next = q_pointer->next;
   q_pointer->next = terminate;    

   return split;   
}

void build_work_queue(DIR* directory, queue_element_t* element, queue_t* queue, struct dirent* entry, struct dirent* result, int status){

   queue_element_t *new_element;

	
	directory = opendir(element->path_name);
	if(directory == NULL){
		printf("Unable to open directory %s \n", element->path_name);
		return;
	}

	while(1){
		status = readdir_r(directory, entry, &result); /* Read directory entry. */
		if(status != 0){
		 	printf("Unable to read directory %s \n", element->path_name);
			break;
		 }
			
	  	 if(result == NULL)				/* End of directory. */
		 	break; 										  											  
				
		 if(strcmp(entry->d_name, ".") == 0)	/* Ignore the "." and ".." entries. */
			continue;
				
		 if(strcmp(entry->d_name, "..") == 0)
			continue;

		 /* Insert this directory entry in the queue. */
		 new_element = (queue_element_t *)malloc(sizeof(queue_element_t));
		 if(new_element == NULL){
		 	printf("Error allocating memory. Exiting. \n");
			exit(-1);
		 }
				
		 /* Construct the full path name for the directory item stored in entry. */
	 	 strcpy(new_element->path_name, element->path_name);
		 strcat(new_element->path_name, "/");
		 strcat(new_element->path_name, entry->d_name);
       //printf("Adding to queue %s\n", new_element->path_name);
		 insertElement(queue, new_element);
	}
	
	closedir(directory);

	return;
}

void* static_thread(void *argument){

            static_data *t_data = (static_data *)argument;
            queue_element_t *file_head = t_data->file_head;
            char *str = t_data->search_string;

				FILE *file_to_search;
				char buffer[MAX_LENGTH];
				char *bufptr, *searchptr, *tokenptr;
				int file_occurences = 0;
			
	         queue_element_t *element = malloc(sizeof(queue_element_t));

            while(file_head != NULL){
               element = file_head;
               if(strcmp(file_head->path_name, "EXIT") == 0){ break; }

				   //printf("%s is a regular file. \n", element->path_name);
				   // Search the file for the search string provided as the command-line argument.
				   file_to_search = fopen(element->path_name, "r");
				   if(file_to_search == NULL){
					   printf("Unable to open file %s \n", element->path_name);
                  file_head = file_head->next;
			         continue;	
				   } 
				   else{
					   while(1){
						   bufptr = fgets(buffer, sizeof(buffer), file_to_search);		// Read in a line from the file.
						   if(bufptr == NULL){
							   if(feof(file_to_search)) break;
								   if(ferror(file_to_search)){
									   printf("Error reading file %s \n", element->path_name);
										   break;
								   }
						   }   
									 
						   // Break up line into tokens and search each token.
						   tokenptr = strtok(buffer, " ,.-");
						   while(tokenptr != NULL){
							   searchptr = strstr(tokenptr, str);
							   if(searchptr != NULL){
								   printf("Found string %s within file %s. \n", str, element->path_name);
								   file_occurences ++;
							   }
												
							   tokenptr = strtok(NULL, " ,.-");						// Get next token from the line. 
						   }
					   }
				   }
				   fclose(file_to_search);
               file_head = file_head->next;
            }
            pthread_mutex_lock(&occurrence_mutex);
            total_occurrences += file_occurences;
            pthread_mutex_unlock(&occurrence_mutex);

            return(NULL);
}


int	 							/* Parallel search with dynamic load balancing. */
parallelSearchDynamic(char **argv)
{
   int num_occurrences = 0;
   int num_files = 0;
	queue_element_t *element, *new_element;
   struct stat file_stats;
   int status; 
 
   DIR *directory = NULL;
   struct dirent *result = NULL;
   struct dirent *entry = (struct dirent *)malloc(sizeof(struct dirent) + MAX_LENGTH);

   queue_t *dir_queue = createQueue(); /* Create and initialize the queue data structure. */
   queue_t *file_queue = createQueue();
   element = (queue_element_t *)malloc(sizeof(queue_element_t));

   // If malloc fails exit the program //
	if(element == NULL){
		perror("malloc");
		 exit(EXIT_FAILURE);
  	}

   strcpy(element->path_name, argv[2]);
   element->next = NULL;
   insertElement(dir_queue, element);

   // While there are more elements in the directory queue filter them //
   while(dir_queue->head != NULL){

      // Get File Stats from Queue Element //
	 	queue_element_t *element = removeElement(dir_queue);
	 	status = lstat(element->path_name, &file_stats);

      // If status is -1 there was an error getting file information //
	 	if(status == -1){
			printf("Error obtaining stats for %s \n", element->path_name);
			free((void *)element);
			continue;
	 	}

		// If the file is a symbolic link ignore it //
	 	if(S_ISLNK(file_stats.st_mode)){ 
	 	} 
	 
      // If the file is a directory add its contents to the queue //
	 	else if(S_ISDIR(file_stats.st_mode)){

         /* Add to queue and free previous pointer */
         build_work_queue(directory, element, dir_queue, entry, result, status);
         free((void *)element);
			
	 
		} 

      // If the file is regular then add it to the file queue //
	   else if(S_ISREG(file_stats.st_mode)){

         // Insert to Queue // 
         insertElement(file_queue, element);
			num_files++;

	   }
	   else{
		   printf("%s is of type other. \n", element->path_name);
	   }
  }
   // Create Bucket //
   int num_threads = atoi(argv[3]);
   int bucket = num_files/num_threads;
   int rc;
   int t_num;

   // Assign Threads and Init Mutex //
   pthread_t worker_threads[num_threads];

   size_t length = strlen(argv[1]);
   char* s_string = malloc(length+1);
   strcpy(s_string, argv[1]);
   void *t_status = 0;



   dynamic_data *t_data = malloc(sizeof(dynamic_data));
   t_data->file_queue = file_queue;
   t_data->search_string = s_string;

   //printf("%s\n",t_data->file_queue->head->path_name);

   for(t_num = 0; t_num < num_threads; t_num++){
      rc = pthread_create(&worker_threads[t_num], NULL, dynamic_thread,(void *)t_data);
      if(rc){
         printf("You dun fucked up m9");
         exit(-1);
      }
   }

   for(t_num = 0; t_num < num_threads; t_num++){
      rc = pthread_join(worker_threads[t_num], &t_status);
      if(rc){
         printf("You dun fucked up m9");
         exit(-1);
      }

   }
   pthread_mutex_destroy(&occurrence_mutex);
   pthread_mutex_destroy(&queue_mutex); 
   return total_occurrences;

}

void* dynamic_thread(void *argument){

            dynamic_data *t_data = (dynamic_data *)argument;
            queue_t *file_queue = t_data->file_queue;
            char *str = t_data->search_string;

				FILE *file_to_search;
				char buffer[MAX_LENGTH];
				char *bufptr, *searchptr, *tokenptr;
				int file_occurences = 0;
            char file_path[MAX_LENGTH];
			
	         queue_element_t *element = malloc(sizeof(queue_element_t));
            while(file_queue->head != NULL){
      
               pthread_mutex_lock(&queue_mutex);
               element = removeElement(file_queue);
               strcpy(file_path,element->path_name);
               pthread_mutex_unlock(&queue_mutex);

				  // printf("%s is a regular file. \n", element->path_name);
				   // Search the file for the search string provided as the command-line argument.
				   file_to_search = fopen(file_path, "r");
				   if(file_to_search == NULL){
					   printf("Unable to open file %s \n", file_path);
			         continue;
				   } 
				   else{
					   while(1){
						   bufptr = fgets(buffer, sizeof(buffer), file_to_search);		// Read in a line from the file.
						   if(bufptr == NULL){
							   if(feof(file_to_search)) break;
								   if(ferror(file_to_search)){
									   printf("Error reading file %s \n", file_path);
										   break;
								   }
						   }   
									 
						   // Break up line into tokens and search each token.
						   tokenptr = strtok(buffer, " ,.-");
						   while(tokenptr != NULL){
							   searchptr = strstr(tokenptr, str);
							   if(searchptr != NULL){
								   printf("Found string %s within file %s. \n", str, file_path);
								   file_occurences ++;
							   }
												
							   tokenptr = strtok(NULL, " ,.-");						// Get next token from the line. 
						   }
					   }
				   }
				   fclose(file_to_search);
            }
            pthread_mutex_lock(&occurrence_mutex);
            total_occurrences += file_occurences;
            pthread_mutex_unlock(&occurrence_mutex);

            return(NULL);
}
