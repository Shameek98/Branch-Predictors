#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"
#include <stdint.h>
#include <inttypes.h>
/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/

//Global declarations:

int index_len, tag_len, index_len;
unsigned int addr_index, tag, mask_for_index_gs, mask_for_index_bm;
int hit_index = 0;
int branch_hit = 0;
int i,j;
int predicted_outcome;
unsigned int gbhr;
unsigned int mask_for_gshare;
unsigned int mask_for_gbhr;
unsigned int effective_index, partial_index, partial_index_2;
int gshare_index, bimodal_index;
int predictions, mispredictions;
float misprediction_rate;

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    //allocating and initiating to weakly taken in mem
    //printf("issue with malloc");
    bimodal_predictor *brp = (bimodal_predictor*)malloc((1<<params.M2)*sizeof(bimodal_predictor));
    int *gshare = (int*)malloc((1<<params.M1)*sizeof(int));
    int *hybrid_chooser = (int*)malloc((1<<params.K)*sizeof(int));
    
    for(i=0;i<(1<<params.M2);i++){
	    brp[i].counter = 2;
    }
    for(i=0;i<(1<<params.M1);i++){
	    gshare[i] = 2;
    }
    for(i=0;i<(1<<params.K);i++){
        hybrid_chooser[i] = 1;
    }


    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() faik_for_gshare)ed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    char str[2];
    gbhr = 0;
    mask_for_gshare = (1<<(params.N+2))-1;
    mask_for_gbhr = (1<<params.N)-1;
    predictions =0;
    mispredictions = 0;
    int actual_outcome = 0;	    
        
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        if(outcome == 't')
		    actual_outcome = 1;
        else if(outcome == 'n')
		    actual_outcome = 0;
        /*if(count < 10001){
        if (outcome == 't')
            printf("=%d %lx %s\n", count, addr, "t");           // Print and test if file is read correctly
        else if (outcome == 'n')
            printf("=%d %lx %s\n", count, addr, "n");
        }*/          // Print and test if file is read correctly*/
        /*************************************
            Add branch predictor code here
        **************************************/
        if(strcmp(params.bp_name, "bimodal") == 0){
	    tag_len = 30-(params.M2);  //since lower 2 bits are useless
	    index_len = params.M2;
	    mask_for_index_bm = (1<<(params.M2+2))-1;
	    tag = (addr>>(params.M2+2));
	    bimodal_index = (addr & mask_for_index_bm)>>2;
	    predicted_outcome = predict_bimodal(brp, outcome);
            train_bimodal(brp, outcome);
            predictions++;
        if(predicted_outcome != actual_outcome)
            mispredictions++;
    	    
	}	
        if(strcmp(params.bp_name, "gshare") == 0){
	    mask_for_index_gs = (1<<(params.M1+2))-1;
	    gshare_index = (addr & mask_for_index_gs)>>2;
	    predicted_outcome = predict_gshare(gshare, outcome, params);
        // if(count < 10001)
        //     printf("GP: %d %d\n",gshare_index,gshare[gshare_index]);
        train_gshare(gshare, outcome, params);
    	gshare_update(outcome, params);
        // if(count < 10001)
        //     printf("GU: %d %d\n",gshare_index,gshare[gshare_index]);
	    predictions++;
        if(predicted_outcome != actual_outcome)
            mispredictions++;
        }
        if(strcmp(params.bp_name, "hybrid") == 0){
	    int prediction_bimodal, prediction_gshare, chooser_index; 
	    mask_for_index_bm = (1<<(params.M2+2))-1;
	    mask_for_index_gs = (1<<(params.M1+2))-1;
	    gshare_index = (addr & mask_for_index_gs)>>2;
	    bimodal_index = (addr & mask_for_index_bm)>>2;
	    prediction_bimodal = predict_bimodal(brp, outcome);
	    prediction_gshare = predict_gshare(gshare, outcome, params);
	    chooser_index = (addr>>2) & ((1<<params.K)-1);
	    if(hybrid_chooser[chooser_index]>=2){
		predicted_outcome = prediction_gshare;
		train_gshare(gshare, outcome, params);
	    }
	    else{
		predicted_outcome = prediction_bimodal;
		train_bimodal(brp, outcome);
	    }
	    gshare_update(outcome, params);
	    if(actual_outcome == prediction_bimodal && actual_outcome != prediction_gshare){
		if(hybrid_chooser[chooser_index] != 0)
		    hybrid_chooser[chooser_index]--;
	    } 
	    else if(actual_outcome != prediction_bimodal && actual_outcome == prediction_gshare){
		if(hybrid_chooser[chooser_index] != 3)
		    hybrid_chooser[chooser_index]++;
	    }
        predictions++;
        if(predicted_outcome != actual_outcome)
            mispredictions++;
	}
    }
    misprediction_rate = ((float)mispredictions/predictions)*100;
    if(strcmp(params.bp_name, "bimodal") == 0){
        printf("OUTPUT\n");
            printf("number of predictions: %d\n", predictions);
            printf("number of mispredictions: %d\n", mispredictions);
            printf("misprediction rate: %0.2f%%\n", misprediction_rate);
            printf("FINAL BIMODAL CONTENTS\n");
        for(i=0;i<(1<<params.M2);i++){
            
		    printf("%d   ", i);
		    printf("%d", brp[i].counter);
		    printf("\n");
	    }
    }
    if(strcmp(params.bp_name, "gshare") == 0){
        printf("OUTPUT\n");
            printf("number of predictions: %d\n", predictions);
            printf("number of mispredictions: %d\n", mispredictions);
            printf("misprediction rate: %0.2f%%\n", misprediction_rate);
            printf("FINAL GSHARE CONTENTS\n");
        for(i=0;i<(1<<params.M1);i++){
		    printf("%d   ", i);
		    printf("%d", gshare[i]);
		    printf("\n");
	    }
    }
    if(strcmp(params.bp_name, "hybrid") == 0){
        printf("OUTPUT\n");
            printf("number of predictions: %d\n", predictions);
            printf("number of mispredictions: %d\n", mispredictions);
            printf("misprediction rate: %0.2f%%\n", misprediction_rate);
            printf("FINAL CHOOSER CONTENTS\n");
        for(i=0;i<(1<<params.K);i++){
            printf("%d   ", i);
            printf("%d", hybrid_chooser[i]);
            printf("\n");
        }
        printf("FINAL GSHARE CONTENTS\n");
        for(i=0;i<(1<<params.M1);i++){
		    printf("%d   ", i);
		    printf("%d", gshare[i]);
		    printf("\n");
	    }
        printf("FINAL BIMODAL CONTENTS\n");
        for(i=0;i<(1<<params.M2);i++){
            
		    printf("%d   ", i);
		    printf("%d", brp[i].counter);
		    printf("\n");
	    }
    }
    return 0;
}
int predict_bimodal(bimodal_predictor *brpredict, char out_come){ 
    int prediction = 0;
    if(brpredict[bimodal_index].counter == 1 || brpredict[bimodal_index].counter == 0){
	prediction = 0;
    }
    else if(brpredict[bimodal_index].counter == 2 || brpredict[bimodal_index].counter == 3){
	prediction = 1;
    }
    //train_bimodal(brpredict, out_come);
    return prediction;
}
void train_bimodal(bimodal_predictor *brpredict, char out_come){
    if(out_come == 'n'){
	if(brpredict[bimodal_index].counter != 0){
	    brpredict[bimodal_index].counter--;
	}
   }
    else if(out_come == 't'){
	if(brpredict[bimodal_index].counter != 3){
	    brpredict[bimodal_index].counter++;
	}
    }
}
int predict_gshare(int *gs, char out_come, bp_params param){
    int prediction_gshare = 0;
    partial_index = gbhr^(gshare_index>>(param.M1-param.N));
    partial_index_2 = ((1<<(param.M1-param.N))-1) & gshare_index;
    effective_index = ((partial_index<<(param.M1-param.N)) | partial_index_2);
    if(gs[effective_index] == 1 || gs[effective_index] == 0){
	prediction_gshare = 0;
    }
    else if(gs[effective_index] == 2 || gs[effective_index] == 3){
	prediction_gshare = 1;
    }
    //train_gshare(gs, out_come, param);
    return prediction_gshare;
}
void train_gshare(int *gs, char out_come, bp_params param){   
    if(out_come == 't'){
	if(gs[effective_index] != 3){
            gs[effective_index]++;
	}
    }
    else if(out_come == 'n'){
	if(gs[effective_index] != 0){
	    gs[effective_index]--;
	}
    }
    //gshare_update(out_come, param);
}
void gshare_update(char out_come, bp_params param){  
    int out;
    if(out_come == 't')
	out = 1;
    else if(out_come == 'n')
	out = 0;
    gbhr = (((gbhr>>1)+(out<<(param.N-1))) & mask_for_gbhr);
}

