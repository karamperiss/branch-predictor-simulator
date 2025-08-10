#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <sstream> // std::ostringstream
#include <cmath>   // pow()
#include <cstring> // memset()!
#include "ras.h"

// Add any includes here....
#include <assert.h>     /* assert */
#include <list>

typedef std::list<ADDRINT>::iterator set_iterator_t;


/**
 * A generic BranchPredictor base class.
 * All predictors can be subclasses with overloaded predict(), update(), getNameAndConfig(), getResults()
 * methods.
 **/
class BranchPredictor
{
public:
    BranchPredictor() : correct_predictions(0), incorrect_predictions(0) {};
    ~BranchPredictor() {};

    virtual bool predict(ADDRINT ip, ADDRINT target) = 0;  // target is needed only for some static predictors
    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall=false, bool isRet=false) = 0;
    virtual std::string getNameAndConfig() = 0;
    virtual std::string getResults() = 0;


    void resetCounters() { correct_predictions = incorrect_predictions = 0; };

protected:
    void updateCounters(bool predicted, bool actual) {
        if (predicted == actual)
            correct_predictions++;
        else
            incorrect_predictions++;
    };

    UINT64 correct_predictions;
    UINT64 incorrect_predictions;
};

// ------------------------------------------------------
// NbitPredictor - a generalization of 1,2 bit saturating counters
// ------------------------------------------------------
class NbitPredictor : public BranchPredictor
{
public:
    NbitPredictor(unsigned index_bits_, unsigned cntr_bits_)
        : BranchPredictor(), index_bits(index_bits_), cntr_bits(cntr_bits_) {
        table_entries = 1 << index_bits;
        TABLE = new unsigned int[table_entries];
        memset(TABLE, 0, table_entries * sizeof(*TABLE));
        
        COUNTER_MAX = (1 << cntr_bits) - 1;
        resetCounters();
    };
    ~NbitPredictor() { delete TABLE; };

    virtual bool predict(ADDRINT ip, ADDRINT target) {
        unsigned int ip_table_index = ip % table_entries;
        unsigned long long ip_table_value = TABLE[ip_table_index];
        unsigned long long prediction = ip_table_value >> (cntr_bits - 1);   // get the msb of the counter
        return (prediction != 0);
    };

    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall=false, bool isRet=false) {
        unsigned int ip_table_index = ip % table_entries;
        if (actual) {
            if (TABLE[ip_table_index] < COUNTER_MAX)
                TABLE[ip_table_index]++;
        } else {
            if (TABLE[ip_table_index] > 0)
                TABLE[ip_table_index]--;
        }
        
        updateCounters(predicted, actual);
    };

    virtual std::string getNameAndConfig() {
        std::ostringstream stream;
        stream << "Nbit-" << pow(2.0,double(index_bits)) / 1024.0 << "K-" << cntr_bits;
        return stream.str();
    }

    virtual std::string getResults() {
        std::ostringstream stream;
        stream << "Correct: " << correct_predictions << " Incorrect: "<< incorrect_predictions;
        return stream.str();
    }

private:
    unsigned int index_bits, cntr_bits;
    unsigned int COUNTER_MAX;
    
    unsigned int *TABLE;
    unsigned int table_entries;
};


// ------------------------------------------------------
// Static Not Taken Predictor
// ------------------------------------------------------
class NotTakenPredictor : public BranchPredictor{

public:
    NotTakenPredictor() : BranchPredictor(){
        resetCounters();
    };

    ~NotTakenPredictor(){};

    virtual bool predict(ADDRINT ip, ADDRINT target) override{
        return false;
    };

    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall = false, bool isRet = false) override{
        updateCounters(predicted, actual);
    };

    virtual std::string getNameAndConfig() override{
        return "Static-NotTaken";
    }

    virtual std::string getResults() override{
        std::ostringstream stream;
        stream << "Correct: " << correct_predictions << " Incorrect: " << incorrect_predictions;
        return stream.str();
    }
};

// ------------------------------------------------------
// Static BTFNT Predictor
// ------------------------------------------------------
class BTFNTPredictor : public BranchPredictor{

public: 
    BTFNTPredictor() : BranchPredictor(){
        resetCounters();
    };

    ~BTFNTPredictor(){};

    virtual bool predict(ADDRINT ip, ADDRINT target) override{
        return (target < ip);
    };

    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall = false, bool isRet = false) override{
        updateCounters(predicted, actual);
    };

    virtual std::string getNameAndConfig() override{
        return "Static-BTFNT";
    }

    virtual std::string getResults() override{
        std::ostringstream stream;
        stream << "Correct: " << correct_predictions << " Incorrect: " << incorrect_predictions;
        return stream.str();
    } 

};

// ------------------------------------------------------
// Gshare Predictor
// ------------------------------------------------------
class GsharePredictor : public BranchPredictor{

public:
    GsharePredictor(unsigned bits): BranchPredictor(), bits(bits){
        table_entries = 1 << bits;
        PHT = new unsigned int[table_entries];
        memset(PHT, 0, table_entries * sizeof(*PHT)); //patern history table
        COUNTER_MAX = 3;
        GHR = 0; //global history register
        resetCounters();
    } 

    ~GsharePredictor(){
        delete[] PHT;
    }

    virtual bool predict(ADDRINT ip, ADDRINT target) override{
        unsigned int index = getIndex(ip);
        return (PHT[index] >> 1) != 0; //if MSB of 2-bit counter is 1 predict taken
    }

    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall=false, bool isRet=false) override{
        unsigned int index = getIndex(ip);

        if(actual){
            if (PHT[index] < COUNTER_MAX)
            PHT[index]++; //icrease 2-bit counter
        }
        else{
            if(PHT[index] > 0)
            PHT[index]--; //decrease 2-bit counter
        }

        GHR = ((GHR << 1) | (actual ? 1 : 0)) & ((1 << bits) -1); //update GHR 

        updateCounters(predicted, actual);
    }

    virtual std::string getNameAndConfig() override{
        std::ostringstream stream;
        stream << "Gshare-" << bits << "b";
        return stream.str();
    }

    virtual std::string getResults() override{
        std::ostringstream stream;
        stream << "Correct: " << correct_predictions << " Incorrect: " << incorrect_predictions;
        return stream.str();
    }

private:
    unsigned int getIndex(ADDRINT ip){
        unsigned int ip_bits = (unsigned int)(ip &((1 << bits) - 1));
        return ip_bits ^ GHR;
    }

unsigned int *PHT;
unsigned int table_entries;
unsigned int COUNTER_MAX;
unsigned int GHR;
unsigned int bits;

};

// ------------------------------------------------------
// BTB
// ------------------------------------------------------
class BTBPredictor : public BranchPredictor{
public:
    BTBPredictor(int btb_lines, int btb_assoc, int ras_entries_)
        : table_lines(btb_lines), table_assoc(btb_assoc), ras_entries(ras_entries_),   incorrect_target_predictions(0), incorrect_ras_predictions(0)
    {
        ras = new RAS(ras_entries_);
        num_sets = btb_lines/btb_assoc;
        assert((num_sets & (num_sets-1)) == 0);
        TABLE = new std::list<std::pair<ADDRINT, ADDRINT>>[num_sets];
        resetCounters();
    }

    ~BTBPredictor() {
        delete ras;
        delete[] TABLE;
    }

    virtual bool predict(ADDRINT ip , ADDRINT target) {
        predicted_target = 0;

        int index = (ip >> 2) & (num_sets - 1);
        auto &set = TABLE[index];

        for (auto it = set.begin(); it != set.end(); ++it) {
            if (it->first == ip) {
                set.splice(set.begin(), set, it); //LRU
                predicted_target = it->second;
                return true;
            }
        }
        return false;
    }

    virtual void update(bool predicted, bool actual, ADDRINT ip, ADDRINT target, bool isCall=false, bool isRet=false) {
        updateCounters(predicted, actual);

        int index = (ip >> 2) & (num_sets - 1);
        auto &set = TABLE[index];
        
        if (isCall) {
            ras->push_addr(ip + 4);
        }

        if (isRet) {
            if (!ras->pop_addr_and_check(target)) {
                incorrect_ras_predictions++;
            }
            return;
        }

        auto it = std::find_if(set.begin(), set.end(),
            [ip](const std::pair<ADDRINT, ADDRINT>& entry) {
                return entry.first == ip;
            });

        if (predicted) {
            if (actual) {
                if (predicted_target != target) {
                    incorrect_target_predictions++;
                    if (it != set.end()) {
                        it->second = target;
                    }
                }
            } else {
                if (it != set.end()) {
                    set.erase(it); //remove from BTB
                }
            }
        } else {
            if (actual) {
                // insert to BTB
                if (it == set.end()) {
                    if ((int)set.size() >= table_assoc) {
                        set.pop_back(); //LRU remove
                    }
                    set.push_front({ip, target});
                }
            }
        }
    }

    virtual std::string getNameAndConfig() { 
        std::ostringstream stream;
        stream << "BTB-" << table_lines << "-" << table_assoc << "-" << ras_entries;
        return stream.str();
    }
   
    virtual std::string getResults() {
        std::ostringstream stream;
        stream << "Correct: " << correct_predictions
        << " Incorrect: " << incorrect_predictions
        << " Incorrect_Targets: " << incorrect_target_predictions
        << " Incorrect_RAS: " << incorrect_ras_predictions;
        return stream.str();
    }

private:
    int table_lines, table_assoc;
    int num_sets;
    int ras_entries;

    RAS *ras;   //RAS stack  
    std::list<std::pair<ADDRINT, ADDRINT>> *TABLE;
    ADDRINT predicted_target = 0;

    UINT64 incorrect_target_predictions;
    UINT64 incorrect_ras_predictions;
};

#endif
