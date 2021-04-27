#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){
    FILE *output_ptr;
    output_ptr = fopen("input.txt","w");
    char data;
    srand(time(NULL));
    for(long long int i=1;i<10001;i++){
        data=(rand()%95)+32;
        fprintf(output_ptr,"%c",data);
        if(i%50==0)
            fprintf(output_ptr,"\n");
    }
    fclose(output_ptr);
}