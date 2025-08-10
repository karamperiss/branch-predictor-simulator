#ifndef RAS_H
#define RAS_H

#include <vector>

class RAS
{
public:
    RAS(UINT32 num_entries) 
      : max_entries(num_entries){};
    ~RAS() {};

    void push_addr(ADDRINT addr) {
        addr_vec.push_back(addr);
        if (addr_vec.size() > max_entries)
            addr_vec.erase(addr_vec.begin());   // if full, remove oldest entry
    }

    bool pop_addr_and_check(ADDRINT target) {
        if (addr_vec.empty()) {    // if empty, can't predict correctly
            return false;
        }

        ADDRINT ras_ip = addr_vec.back();
        addr_vec.pop_back();

        return (ras_ip == target);
    }

private:
    UINT32 max_entries;
    std::vector<ADDRINT> addr_vec;
};

#endif
