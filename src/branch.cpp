#include "pin.H"

#include <iostream>
#include <fstream>
#include <cassert>

#include "branch_predictor.h"

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */
KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE,    "pintool",
    "o", "branch.out", "specify output file name");
/* ===================================================================== */

/* ===================================================================== */
/* Global Variables                                                      */
/* ===================================================================== */
std::vector<BranchPredictor *> branch_predictors;
typedef std::vector<BranchPredictor *>::iterator bp_iterator_t;

UINT64 total_instructions;
std::ofstream outFile;

/* ===================================================================== */

INT32 Usage()
{
    std::cerr << "This tool simulates various branch predictors.\n\n";
    std::cerr << KNOB_BASE::StringKnobSummary();
    std::cerr << std::endl;
    return -1;
}

/* ===================================================================== */

VOID count_instruction()
{
    total_instructions++;
}


VOID branch_instruction(ADDRINT ip, ADDRINT target, BOOL taken, BOOL isCall, BOOL isRet)
{
    bp_iterator_t bp_it;
    BOOL pred;

    // For each simulated predictor:
    //    - get the prediction
    //    - update the predictor's state using the outcome
    for (bp_it = branch_predictors.begin(); bp_it != branch_predictors.end(); ++bp_it) {
        BranchPredictor *curr_predictor = *bp_it;
        pred = curr_predictor->predict(ip, target);  // target is to be used only by static predictor
        curr_predictor->update(pred, taken, ip, target, isCall, isRet);
    }
}


VOID Instruction(INS ins, void * v)
{
	// Catch any kind of control-flow instruction
    if (INS_IsBranch(ins) || INS_IsCall(ins) || INS_IsRet(ins))
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)branch_instruction,
                       IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                       IARG_BOOL, INS_IsCall(ins), IARG_BOOL, INS_IsRet(ins),
                       IARG_END);

    // Count each and every instruction
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)count_instruction, IARG_END);
}

/* ===================================================================== */

VOID Fini(int code, VOID * v)
{
    bp_iterator_t bp_it;

    // Report total instructions and total cycles
    outFile << "Total Instructions: " << total_instructions << "\n";
    outFile << "\n";

    outFile <<"Branch Predictors: (Name - Results)\n";
    for (bp_it = branch_predictors.begin(); bp_it != branch_predictors.end(); ++bp_it) {
        BranchPredictor *curr_predictor = *bp_it;
        // WARNING:
        // The plot script depends on how this is formatted!
        outFile << "  " << curr_predictor->getNameAndConfig() << ": "
                << curr_predictor->getResults() << "\n";
    }

    outFile.close();
}

/* ===================================================================== */

VOID InitPredictors()
{
    NbitPredictor *n1bitPred = new NbitPredictor(14, 1);
    branch_predictors.push_back(n1bitPred);
    NbitPredictor *n2bitPred = new NbitPredictor(13, 2);
    branch_predictors.push_back(n2bitPred);
    //Not Taken Predictor
    NotTakenPredictor *nottakenPred = new NotTakenPredictor();
    branch_predictors.push_back(nottakenPred);
    //btfnt
    BTFNTPredictor *btfntPred = new BTFNTPredictor();
    branch_predictors.push_back(btfntPred);
    //Gshare
    GsharePredictor *gSharePred = new GsharePredictor(13);
    branch_predictors.push_back(gSharePred);
    //BTB
    BTBPredictor *testBTB= new BTBPredictor(512, 1, 16);
    branch_predictors.push_back(testBTB);
    BTBPredictor *testBTB2= new BTBPredictor(64, 8, 16);
    branch_predictors.push_back(testBTB2);
}


int main(int argc, char *argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc,argv))
        return Usage();

    // Open output file
    outFile.open(KnobOutputFile.Value().c_str());

    // Initialize predictors
    InitPredictors();

    // Instrument function 
    INS_AddInstrumentFunction(Instruction, 0);

    // Called when the instrumented application finishes its execution
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
