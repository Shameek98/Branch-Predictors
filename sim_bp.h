#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement


typedef struct bimodal_predictor{
    int counter;
    unsigned int branch_tag;
}bimodal_predictor;

int predict_bimodal(bimodal_predictor *brpredict, char out_come); 
void train_bimodal(bimodal_predictor *brpredict, char out_come);
int predict_gshare(int *gs, char out_come, bp_params param);
void gshare_update(char out_come, bp_params param); 
void train_gshare(int *gs, char out_come, bp_params param);
//void gshare_update(char out_come); 
#endif
