# Branch-Predictors
Constructed a branch predictor simulator and used it to evaluate different configurations of branch predictors. Modeled a bimodal, gshare and a hybrid branch predictor(combination of bimodal and gshare that conditionally selects between the gshare and bimodal predictors).

Modelled a gshare branch predictor with parameters {m, n}, where:
- m is the number of low-order PC bits used to form the prediction table index.
- n is the number of bits in the global branch history register.

Also modelled a hybrid predictor that selects between the bimodal and the gshare predictors, using a chooser table of 2k 2-bit counters.

Command-line arguments to the simulator:
- To simulate a bimodal predictor: sim bimodal <M2> <tracefile> (where M2 is the number of PC bits used to index the bimodal table)
- To simulate a gshare predictor: sim gshare <M1> <N> <tracefile> (where M1 and N are the number of PC bits and global branch history register bits used to index the gshare table, respectively.)
- To simulate a hybrid predictor: sim hybrid <K> <M1> <N> <M2> <tracefile> (where K is the number of PC bits used to index the chooser table, M1 and N are the number of PC bits and global branch history register bits used to index the gshare table (respectively), and M2 is the number of PC bits used to index the bimodal table.)
