#include "MKSEUser.h"

MKSEUser::MKSEUser(MKSEServer* server, unsigned char* masterKey) {
    this->server = server;
    srand(time(NULL));
    userID = rand();
    memset(iv, 0x00, AES_KEY_SIZE);
    memcpy(key, masterKey, AES_KEY_SIZE);
}

MKSEUser::~MKSEUser() {
}

vector<int> MKSEUser::search(string keyword) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    prf_type kprime;
    Utilities::encrypt((unsigned char*) k_w.data(), AES_KEY_SIZE - 1, key, iv, (unsigned char*) kprime.data());
    Result response = server->search(kprime);
    searchCommunicationSize += response.values.size() * (AES_KEY_SIZE + 4)+AES_KEY_SIZE;

    return response.ids;
}