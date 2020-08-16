#include "MKSEServer.h"

MKSEServer::MKSEServer(int maxCounterSize) {
    memset(iv, 0x00, AES_KEY_SIZE);
    maxCounter = maxCounterSize;
}

MKSEServer::~MKSEServer() {
}

int MKSEServer::getRofFile(int fileId) {
    if (rs.count(fileId) != 0) {
        return rs[fileId];
    }
    return 0;
}

prf_type MKSEServer::update(int r, prf_type cipher, int fileid) {
    if (DictW.count(r) == 0) {
        DictW[r] = unordered_map<prf_type, pair<int, int>, Hasher>();
    }
    DictW[r][cipher] = pair<int, int>(fileid, counter);
    if (rs.count(fileid) == 0) {
        rs[fileid] = r;
    }
    counter++;
    return cipher;
}

Result MKSEServer::search(prf_type q) {
    Result result;
    for (auto item : DictW) {
        int r = item.first;
        prf_type rcipher, dcipher;
        memset(rcipher.data(), 0, AES_KEY_SIZE);
        memcpy(rcipher.data(), &r, sizeof (int));
        Utilities::encrypt((unsigned char*) rcipher.data(), AES_KEY_SIZE - 1, q.data(), iv, (unsigned char*) dcipher.data());
        if (DictW[r].count(dcipher) != 0) {
            result.ids.push_back(DictW[r][dcipher].first);
            result.values.push_back(dcipher);
        }
    }
    return result;
}

vector<int> MKSEServer::search1(prf_type q) {
    vector<int> result;
    for (auto item : DictW) {
        int r = item.first;
        prf_type rcipher, dcipher;
        memset(rcipher.data(), 0, AES_KEY_SIZE);
        memcpy(rcipher.data(), &r, sizeof (int));
        Utilities::encrypt((unsigned char*) rcipher.data(), AES_KEY_SIZE - 1, q.data(), iv, (unsigned char*) dcipher.data());
        if (DictW[r].count(dcipher) != 0) {
            result.push_back(DictW[r][dcipher].first);
        }
    }
    return result;
}

bool MKSEServer::doesRExit(int r) {
    for(std::map<int,int>::iterator it = rs.begin();it!=rs.end();it++) {
        if(it->second==r){
            return true;
        }
    }
    return false;
}
