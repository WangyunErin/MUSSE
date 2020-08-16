#ifndef MKSESERVER_H
#define MKSESERVER_H
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "utils/Utilities.h"
#include "utils/MerkleTree.h"
#include <unordered_map>
#include <utility>

using namespace std;

class Result {
public:
    vector<prf_type> values;
    vector<int> ids;
};

struct Hasher {

    std::size_t operator()(const prf_type &key) const {
        std::hash<uint8_t> hasher;
        size_t result = 0;
        for (size_t i = 0; i < AES_KEY_SIZE; ++i) {
            result = (result << 1) ^ hasher(key[i]);
        }
        return result;
    }
};

class MKSEServer {
private:
    map<int, unordered_map<prf_type, pair<int, int>, Hasher> > DictW;
    map<int, int> rs;
    unsigned char iv[AES_KEY_SIZE];
    int counter = 0;
    int maxCounter;

public:
    MKSEServer(int maxCounterSize);
    virtual ~MKSEServer();
    int getRofFile(int fileId);
    prf_type update(int r, prf_type cipher, int fileid);
    Result search(prf_type q);
    vector<int> search1(prf_type q);
    bool doesRExit(int r);


};

#endif /* MKSESERVER_H */

