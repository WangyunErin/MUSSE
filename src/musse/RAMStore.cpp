#include "RAMStore.hpp"
#include <iostream>
#include "ORAM.hpp"
using namespace std;

RAMStore::RAMStore(size_t count)
: store(count) {
}

RAMStore::~RAMStore() {
}

block RAMStore::Read(int pos) {
    return store[pos];
}

void RAMStore::Write(int pos, block b) {
    store[pos] = b;
}
