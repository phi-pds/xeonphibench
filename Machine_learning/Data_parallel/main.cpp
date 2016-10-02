#include <iostream>
#include <random>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "learner.h"

#define TRAIN_SIZE 60000
#define TEST_SIZE 10000

#define MINI_BATCH_SIZE 200
#define EPOCH 1

/* 		Time checker		*/
#define START_TIME gettimeofday(&start_time, NULL);
#define END_TIME gettimeofday(&end_time, NULL);
#define PRINT_TIME  timersub(&end_time, &start_time, &elapsed_time); \
                    printf("%ld.%d\n", exec_time.tv_sec, exec_time.tv_usec);


/*		MNIST file path		*/
#define TRAIN_PATH1     "./Mnist/train-images.idx3-ubyte"
#define TRAIN_PATH2     "./Mnist/train-labels.idx1-ubyte"
#define TEST_PATH1      "./Mnist/t10k-images.idx3-ubyte"
#define TEST_PATH2      "./Mnist/t10k-labels.idx1-ubyte"

void argscheck(int ac);

struct timeval start_t, end_t, sum_t[10], exec_t;

typedef unsigned char byte;

static double True = 0;
static double False = 0;

static void train(Net* net);
static void test(Net* net);
static void error_rate(double* result, double* desired);
static void report(char* num_thread, char* file);

struct timeval start_time, end_time, elapsed_time, exec_time;

int main(int ac, char* av[])
{
    int layer_size[] = {INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE};

    argscheck(ac);

    Net* net = new Net(layer_size, NUM_LAYER, MINI_BATCH_SIZE, EPOCH, atoi(av[1]));

    // train
    train(net);

    // test
    test(net);

    // report
    report(av[1], av[2]);

    return 0;
}

void argscheck(int ac)
{
    if (ac < 2) {
        printf("\tPlease check the inputs of program \n");
        printf("\tARG1: number of threads\n");
        printf("\tARG2: name of file which report the result of network\n");

        exit(1);
    }
}

void train(Net *net)
{
    int i, j, k, l;
    FILE *inf, *outf;

    byte input[INPUT_SIZE];
    byte output[OUTPUT_SIZE];

    double input_double[MINI_BATCH_SIZE][INPUT_SIZE];
    double output_double[MINI_BATCH_SIZE][OUTPUT_SIZE];

    if ((inf = fopen(TRAIN_PATH1, "rb")) == NULL) {
        printf("%s open failed\n", TRAIN_PATH1);
        printf("please run at the root of project\n");
        exit(1);
    }

    if ((inf = fopen(TRAIN_PATH2, "rb")) == NULL) {
        printf("%s open failed\n", TRAIN_PATH2);
        printf("please run at the root of project\n");
        exit(1);
    }


    for(i = 0; i < EPOCH; i++){
        printf("epoch %d is running...\n", i+1);
        fread(input, sizeof(byte), 16, inf);	// trash
        fread(output, sizeof(byte), 8, outf);	// trash

        for(j = 0; j < TRAIN_SIZE/MINI_BATCH_SIZE; j++){ 		// undiviable -> cutting...
            for(k=0; k<MINI_BATCH_SIZE; k++){
                fread(input, sizeof(byte), INPUT_SIZE, inf);
                fread(output, sizeof(byte), 1, outf);
                for(l = 0; l < INPUT_SIZE; l++)
                    input_double[k][l] = (double)input[l]!=0?1:0;
                for(l = 0; l < OUTPUT_SIZE; l++)
                    output_double[k][l] = (double)((1 << output[0]) & (1 << l))!=0?1:0;
            }
            START_TIME
            net->train(input_double, output_double, MINI_BATCH_SIZE);
			END_TIME
            timersub(&end_time, &start_time, &elapsed_time);
            timeradd(&elapsed_time, &exec_time, &exec_time);
        }
        rewind(inf);
        rewind(outf);
    }
}

void test(Net *net){
    int i, l;
    double *result;
    FILE *inf, *outf;

    byte input[INPUT_SIZE];
    byte output[OUTPUT_SIZE];

    double input_double[INPUT_SIZE];
    double output_double[OUTPUT_SIZE];

    if ((inf = fopen(TRAIN_PATH1, "rb")) == NULL) {
        printf("%s open failed\n", TRAIN_PATH1);
        printf("please run at the root of project\n");
        exit(1);
    }

    if ((inf = fopen(TRAIN_PATH2, "rb")) == NULL) {
        printf("%s open failed\n", TRAIN_PATH2);
        printf("please run at the root of project\n");
        exit(1);
    }

    fread(input, sizeof(byte), 16, inf);	// trash
    fread(output, sizeof(byte), 8, outf);	// trash

    puts("Testing...");
    for(i = 0; i < TEST_SIZE; i++){
        fread(input, sizeof(byte), INPUT_SIZE, inf);
        fread(output, sizeof(byte), 1, outf);

        for(l = 0; l < INPUT_SIZE; l++)
            input_double[l] = (double)input[l]!=0?1:0;
        for(l = 0; l < OUTPUT_SIZE; l++)
            output_double[l] = (double)((1 << output[0]) & (1 << l))!=0?1:0;

        result = net->test(input_double);

        error_rate(result, output_double);
    }
}

void error_rate(double result[], double desired[])
{
    double max = -1;
    int maxit = -1;

    for(int i=0; i<OUTPUT_SIZE; ++i)
        if(result[i] > max)
            max = result[i], maxit = i;

    for(int i=0; i<OUTPUT_SIZE; ++i)
        (i == maxit)? result[i] = 1: result[i] = 0;

    bool checker = true;
    for(int j=0; j<OUTPUT_SIZE; j++)
        if(result[j] != desired[j])
            checker = false;
    (checker)? True++: False++;
}

void report(char* num_thread, char* file)
{
    puts("==========Result Report==========");
    printf("# of thread : %s\n", num_thread);
    if(True+False)
        printf("recognition rate : %lf %\n", True/(True+False)*100);
    printf("# of True : %d\n", (int)True);
    printf("# of False : %d\n", (int)False);
    printf("execution time : ");
    PRINT_TIME
    for(int i=0; i<3; i++)
        printf("section [%d] : %ld.%d\n", i, sum_t[i].tv_sec, sum_t[i].tv_usec);

    FILE* outf = fopen(file, "w+");
    char str[256];
    sprintf(str, "# of thread : %s\n", num_thread);
    sprintf(str, "%s mini batch size : %d\n", str, MINI_BATCH_SIZE);
    sprintf(str, "%s epoch : %d\n", str, EPOCH);
    sprintf(str, "%s learning rate : %lf\n", str, LEARNING_RATE);
    sprintf(str, "%s recognition rate : %lf %\n", str, True/(True+False)*100);
    sprintf(str, "%s execution time : %ld.%ld\n", str, exec_time.tv_sec, exec_time.tv_usec);
    for(int i=0; i<3; i++)
        sprintf(str, "%s section [%d] : %ld.%d\n", str, i, sum_t[i].tv_sec, sum_t[i].tv_usec);
    fprintf(outf, "%s", str);
}
