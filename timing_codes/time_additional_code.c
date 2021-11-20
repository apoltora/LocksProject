#include <time.h>
#include <stdio.h>


 // DONT USE THIS!!! Use the other code in this folder
 // This has lesser resolution measurement
// Just added this as additional code

int main(){
    clock_t start = clock();

    	// Timed code
		long int temp = 230000000;
		while(temp)
		temp--;
		
    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
    printf("Time elapsed in ms: %f", elapsed);
}
