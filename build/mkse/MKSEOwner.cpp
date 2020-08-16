#include "MKSEOwner.h"

MKSEOwner::~MKSEOwner() {
}

MKSEOwner::MKSEOwner(MKSEServer* server, unsigned char* masterKey, MKSEOwnerRunner* runner) {
    memset(iv, 0x00, AES_KEY_SIZE);
    memcpy(key, masterKey, AES_KEY_SIZE);
    this->server = server;
    this->runner = runner;
    userID = rand();
}

void MKSEOwner::registerUser(int curUserID, unsigned char* key) {
    keys[curUserID] = new unsigned char[AES_KEY_SIZE];
    memcpy(keys[curUserID], key, AES_KEY_SIZE);
}

prf_type  MKSEOwner::sharerequest(string keyword, int ind, int userID, int r) { 
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    prf_type kprime, rcipher, dcipher;
    memset(rcipher.data(), 0, AES_KEY_SIZE);
    memcpy(rcipher.data(), &r, sizeof (int));
    Utilities::encrypt((unsigned char*) k_w.data(), AES_KEY_SIZE - 1, keys[userID], iv, (unsigned char*) kprime.data());
    Utilities::encrypt((unsigned char*) rcipher.data(), AES_KEY_SIZE - 1, kprime.data(), iv, (unsigned char*) dcipher.data());
    Utilities::decrypt((unsigned char*) kprime.data(), AES_KEY_SIZE, keys[userID], iv, (unsigned char*) k_w.data());   
    return dcipher; 
}

void MKSEOwner::shareData(string keyword, int ind, int userID) {
    prf_type k_w;
    int r = server->getRofFile(ind);
    if (server->getRofFile(ind) == 0) {
        do {
            r = rand();
        } while (server->doesRExit(r));
    }
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    prf_type kprime, rcipher, dcipher;
    memset(rcipher.data(), 0, AES_KEY_SIZE);
    memcpy(rcipher.data(), &r, sizeof (int));
    Utilities::encrypt((unsigned char*) k_w.data(), AES_KEY_SIZE - 1, keys[userID], iv, (unsigned char*) kprime.data());
    Utilities::encrypt((unsigned char*) rcipher.data(), AES_KEY_SIZE - 1, kprime.data(), iv, (unsigned char*) dcipher.data());
    prf_type result = server->update(r, dcipher, ind);
    Utilities::decrypt((unsigned char*) kprime.data(), AES_KEY_SIZE, keys[userID], iv, (unsigned char*) k_w.data());

    updateCommunicationSize += (AES_KEY_SIZE + 8);
}

void MKSEOwner::beginSetup() {

}

void MKSEOwner::endSetup() {

}