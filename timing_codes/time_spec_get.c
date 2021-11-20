 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
  
 // to compile: gcc -std=gnu11 time_spec_get.c (the std flag is important)

 // looks like this can measure time to higher resolution

 int main(void) {
     struct timespec t0, t1;
 
      if(timespec_get(&t0, TIME_UTC) != TIME_UTC) {
          printf("Error in calling timespec_get\n");
         exit(EXIT_FAILURE);
     }
 
     // Do some work ...
 		long int temp = 230000000;
		while(temp)
		temp--;
		
		
     if(timespec_get(&t1, TIME_UTC) != TIME_UTC) {
         printf("Error in calling timespec_get\n");
         exit(EXIT_FAILURE);
     }
 
     // Calculate the elapsed time
     double diff = (double)(t1.tv_sec - t0.tv_sec) + ((double)(t1.tv_nsec - t0.tv_nsec)/1000000000L);
     double diff_ms = diff*1000;
     printf("Elapsed time: %lf milliseconds\n", diff_ms);
 }
