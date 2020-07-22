#include "OMAPBasedUser.h"
#include "../utils/Utilities.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include "Types.hpp"
#include "MUSSEUserRunner.h"

using namespace std;

OMAPBasedUser::OMAPBasedUser(Server* server, unsigned char* masterKey, int maxKeywordSize, int ownerID, MusesUserRunner* runner) {
    this->server = server;
    this->runner = runner;
    memset(iv, 0x00, AES_KEY_SIZE);
    memcpy(key, masterKey, AES_KEY_SIZE);
    userID = rand();

    this->ownerID = ownerID;
    if (server == NULL) {
        omap = new OMAP(maxKeywordSize, omapkey, runner, userID);
    } else {
        omap = new OMAP(maxKeywordSize, omapkey, server, userID);
    }

}

OMAPBasedUser::~OMAPBasedUser() {
}

prf_type OMAPBasedUser::searchRequest(string keyword, vector<prf_type>& KList) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int fileCnt = getFileCnt(k_w);

    if (fileCnt == -1) {
        return k_w;
    }

    KList.reserve(fileCnt);

    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, i, rnd.data(), key);
        KList.push_back(rnd);
    }
    return k_w;
}

void OMAPBasedUser::searchProcess(vector<prf_type> values, prf_type k_w, vector<int>& finalRes) {

    finalRes.reserve(values.size());
    int cnt = 1;

    for (unsigned int i = 0; i < values.size(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, cnt, tmp.data(), key);
        prf_type decodedString = values[i];

        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        finalRes.push_back(plaintext);
        cnt++;
    }
}

vector<int> OMAPBasedUser::search(string keyword) {
    vector<int> finalRes;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int fileCnt = getFileCnt(k_w);
    
    if (fileCnt == -1) {
        return finalRes;
    }

    KList.reserve(fileCnt);
    finalRes.reserve(fileCnt);
    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, i, rnd.data(), key);
        KList.push_back(rnd);
    }

    std::cout<<"KList generated"<<std::endl;
    std::vector<prf_type> response = server->search(KList);

    searchCommunicationSize += response.size() * AES_KEY_SIZE * 2;

    int cnt = 1;

    for (unsigned int i = 0; i < response.size(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, cnt, tmp.data(), key);
        prf_type decodedString = response[i];

        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        finalRes.push_back(plaintext);
        cnt++;
    }

    return finalRes;
}

prf_type OMAPBasedUser::bitwiseXOR(int input1, int op, prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    result[4] = input2[4] ^ (op & 0xFF);
    for (int i = 5; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type OMAPBasedUser::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void OMAPBasedUser::getAESRandomValue(unsigned char* keyword, int op, int fileCnt, unsigned char* result, unsigned char* curkey) {
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    Utilities::encrypt((unsigned char*) keyword, AES_KEY_SIZE - 1, curkey, iv, result);
}

int OMAPBasedUser::getFileCnt(prf_type input) {
    std::array< uint8_t, ID_SIZE> mapkey;
    std::fill(mapkey.begin(), mapkey.end(), 0);
    std::copy(input.begin(), input.end(), mapkey.begin());
    Bid keyBid(mapkey);
    omap->CommunicationSize = 0;
    string fileCntStr = omap->find(keyBid);
    searchCommunicationSize += omap->CommunicationSize;
    if (fileCntStr != "") {
        return stoi(fileCntStr);
    }
    return -1;
}

void OMAPBasedUser::setFileCnt(prf_type input, int cnt) {
    std::array< uint8_t, ID_SIZE> mapkey;
    std::fill(mapkey.begin(), mapkey.end(), 0);
    std::copy(input.begin(), input.end(), mapkey.begin());
    Bid keyBid(mapkey);

    omap->insert(keyBid, to_string(cnt));
}

int OMAPBasedUser::incFileCnt(prf_type input) {
    std::array< uint8_t, ID_SIZE> mapkey;
    std::fill(mapkey.begin(), mapkey.end(), 0);
    std::copy(input.begin(), input.end(), mapkey.begin());
    Bid keyBid(mapkey);
    string value = omap->incrementFileCnt(keyBid);
    return stoi(value);
}