#include "Prac2.h"

extern float data [SAMPLE_COUNT];
extern float carrier[SAMPLE_COUNT];

__fp16 result [SAMPLE_COUNT];

int main(int argc, char**argv){
    printf("Running Unthreaded Test\n");
    printf("Precision sizeof %d\n", sizeof(float));

__fp16 data2[SAMPLE_COUNT];
__fp16 carrier2[SAMPLE_COUNT];

for (int i = 0;i<SAMPLE_COUNT;i++ ){
	data2[i] = data[i];
	carrier2[i] = carrier[i];
}

    tic(); // start the timer
    for (int i = 0;i<SAMPLE_COUNT;i++ ){
        result[i] = data2[i] * carrier2[i];
    }
    double t = toc();
    printf("Time: %lf ms\n",t/1e-3);
    printf("End Unthreaded Test\n");
    return 0;
}
