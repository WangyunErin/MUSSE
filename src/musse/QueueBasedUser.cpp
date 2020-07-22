#include "QueueBasedUser.h"
#include "../utils/Utilities.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include "Types.hpp"
#include "MUSSEUserRunner.h"

using namespace std;

QueueBasedUser::QueueBasedUser(Server* server, unsigned char* masterKey, int ownerID, MusesUserRunner* runner) {
    this->server = server;
    this->runner = runner;
    memset(iv, 0x00, AES_KEY_SIZE);
    memcpy(key, masterKey, AES_KEY_SIZE);
    userID = rand();
    this->ownerID = ownerID;
}

QueueBasedUser::~QueueBasedUser() {
}

void QueueBasedUser::getAESRandomValue(unsigned char* keyword, int op, int fileCnt, unsigned char* result, unsigned char* curkey) {
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    Utilities::encrypt((unsigned char*) keyword, AES_KEY_SIZE - 1, curkey, iv, result);
}

prf_type QueueBasedUser::searchRequest(string keyword, vector<prf_type>& KList) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    if (FileCnt.find(k_w) == FileCnt.end()) {
        return k_w;
    }
    int fileCnt = FileCnt[k_w];

    KList.reserve(fileCnt);

    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, i, rnd.data(), key);
        KList.push_back(rnd);
    }
    return k_w;
}

void QueueBasedUser::searchProcess(vector<prf_type> values, prf_type k_w, vector<int>& finalRes) {

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


vector<int> QueueBasedUser::search(string keyword) {
    vector<int> finalRes;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    if (FileCnt.find(k_w) == FileCnt.end()) {
        return finalRes;
    }
    int fileCnt = FileCnt[k_w];

    KList.reserve(fileCnt);
    finalRes.reserve(fileCnt);
    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, i, rnd.data(), key);
        KList.push_back(rnd);
    }

    vector<prf_type> response = server->search(KList);

    searchCommunicationSize += response.size() * AES_KEY_SIZE * 2;

    int cnt = 1;

    for (unsigned int i = 0; i < response.size(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, cnt, tmp.data(), key);
        prf_type decodedString = response[i];

        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        finalRes.emplace_back(plaintext);
        cnt++;
    }

    return finalRes;
}

vector<int> QueueBasedUser::search1(string keyword) {

    vector<int> finalRes;
    vector<prf_type> FList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    prf_type notfound;
    memset(notfound.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    for (unsigned int i = 1; server->keepsearching==1; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, i, rnd.data(), key);
        prf_type value = server->search(rnd);
        if(value !=notfound ){
            FList.emplace_back(value);
        }
    }

    searchCommunicationSize += (FList.size()+1) * AES_KEY_SIZE *2;

    int cnt=1;
    for (unsigned int i = 0; i < FList.size(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, cnt, tmp.data(), key);
        prf_type decodedString = FList[i];

        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        finalRes.emplace_back(plaintext);
        cnt++;
    }

    return finalRes;
}

prf_type QueueBasedUser::bitwiseXOR(int input1,  prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    for (int i = 4; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type QueueBasedUser::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}
