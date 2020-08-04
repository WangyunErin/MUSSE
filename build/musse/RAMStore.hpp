#pragma once
#include <map>
#include <array>
#include <vector>

using namespace std;

using byte_t = uint8_t;
using block = std::vector<byte_t>;

class RAMStore {
    std::vector<block> store;

public:
    RAMStore(size_t num);
    ~RAMStore();

    block Read(int pos);
    void Write(int pos, block b);

};
