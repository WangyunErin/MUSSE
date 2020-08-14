#include "Owner.h"
#include "../utils/Utilities.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <time.h>
#include "OwnerRunner.h"

using namespace std;

Owner::Owner(Server* server, unsigned char* masterKey, MusesOwnerRunner* runner) {//, bool useHyperLedgerFabric, string fabricRESTServer
    this->server = server;  //jiang shuru de server and runner pointer fuzhi gei ben duixiang (owner duixiang) de chengyuanbianliang server and runner pointer
    this->runner = runner;
	memset(iv, 0x00, AES_KEY_SIZE);  //memeset neicunfuhzihanshu, initialize byte by byte, jiang 0x00 fugei iv tianchong AES_KEY_SIZE ge zijie
    memcpy(key, masterKey, AES_KEY_SIZE); //mudi address, source address, # of bytes to be copied
    srand(time(NULL));
    userID = rand();
}

Owner::~Owner() {
}

int Owner::updateRequest(string keyword, int ind, prf_type& addr, prf_type& val, QueueBasedUser* user) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE); //.data()function returns the address of array k_w (the address of its first element)
    copy(keyword.begin(), keyword.end(), k_w.data());//copy the elements of keyword to k_w,keyword has 32 bytes

    userID = user->userID;
    int fileCnt = 0;
   //for first and second
    if (allFileCnt[userID].find(k_w) == allFileCnt[userID].end()) {
        allFileCnt[userID][k_w] = 1;
        fileCnt = 1;
    } else {
        allFileCnt[userID][k_w]++;
        fileCnt = allFileCnt[userID][k_w];
    }
    

    prf_type rnd;
    getAESRandomValue(k_w.data(), 0, fileCnt, addr.data(), keys[userID]);
    getAESRandomValue(k_w.data(), 1, fileCnt, rnd.data(), keys[userID]);
    val = bitwiseXOR(ind, rnd);

    if(!setupMode && ForFU){
        if (user->FileCnt.find(k_w) == user->FileCnt.end()) {
            user->FileCnt[k_w] = 1;
        } else {
            user->FileCnt[k_w]++;
        }
        //updateCommunicationSize += (AES_KEY_SIZE+4);//4 is int size (4 bytes),update the user's Filecnt, k_w is AES_KEY_SIZE, cnt is 4
    }
    return 0;
}

int Owner::updateRequest(string keyword, int ind, prf_type& addr, prf_type& val, int userID) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE); //.data()function returns the address of array k_w (the address of its first element)
    copy(keyword.begin(), keyword.end(), k_w.data());//copy the elements of keyword to k_w,keyword has 32 bytes
    //cout<<"start omap access:"<<endl;
    int fileCnt = 0;
    if (omaps.count(userID) != 0) {//count returns the number of element that is searched (userID):0/1, since there is no repeated element in a map
        std::array< uint8_t, ID_SIZE> mapkey;
        std::fill(mapkey.begin(), mapkey.end(), 0);
        std::copy(k_w.begin(), k_w.end(), mapkey.begin());//copy the elements of k_w to mapkey, i.e.,keyword
        Bid keyBid(mapkey);//copy mapkey to object keyBid de chengyuanbianliang id(also an array)
        string value;  //used to store counter
        if (setupMode) {
            if (setupPairs[userID].count(mapkey) == 0) {
                setupPairs[userID][mapkey] = "1";
            } else {
                setupPairs[userID][mapkey] = to_string(stoi(setupPairs[userID][mapkey]) + 1);
            }
            value = setupPairs[userID][mapkey];
            sharepairs[keyBid]=value;
        } else {
            //cout<<"increment filecnt start"<<endl;
            value = omaps[userID]->incrementFileCnt(keyBid);
            //cout<<"incrementcnt finished"<<endl;
        }
        fileCnt = stoi(value);
    }

    prf_type rnd;
    getAESRandomValue(k_w.data(), 0, fileCnt, addr.data(), keys[userID]);
    getAESRandomValue(k_w.data(), 1, fileCnt, rnd.data(), keys[userID]);
    val = bitwiseXOR(ind, rnd);

    return 0;
}

pair<prf_type,prf_type> Owner::share(string keyword, int ind, QueueBasedUser* user) {
    if (accessList.count(user->userID) == 0) {
        accessList[user->userID] = set<int>();
    }
    accessList[user->userID].insert(ind);

    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    int fileCnt = 0;

    if (allFileCnt[user->userID].find(k_w) == allFileCnt[user->userID].end()) {
        allFileCnt[user->userID][k_w] = 1;
        fileCnt = 1;
    } else {
        allFileCnt[user->userID][k_w]++;
        fileCnt = allFileCnt[user->userID][k_w];
    }

    prf_type addr, rnd;
    getAESRandomValue(k_w.data(), 0, fileCnt, addr.data(), keys[user->userID]);
    getAESRandomValue(k_w.data(), 1, fileCnt, rnd.data(), keys[user->userID]);

    prf_type val = bitwiseXOR(ind, rnd);
    if (!setupMode) {
        server->update(addr, val);
        if(ForFU){
            if (user->FileCnt.find(k_w) == user->FileCnt.end()) {
                user->FileCnt[k_w] = 1;
            } else {
                user->FileCnt[k_w]++;
            }
            updateCommunicationSize += (AES_KEY_SIZE+4);//4 is int size (4 bytes),update the user's Filecnt, k_w is AES_KEY_SIZE, cnt is 4
        }
    }

    pair<prf_type,prf_type> keyval(addr,val);

    updateCommunicationSize += 2 * AES_KEY_SIZE;
    return keyval;
}

void Owner::unshare(int ind, QueueBasedUser* user, const TC<int>* testCase){
    int userID=user->userID;
    accessList[userID].clear();
    allFileCnt[userID].clear();
    beginSetup();
    unsigned char newuserKey[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        newuserKey[i] = rand() % 256;
    }
    registerUser(userID, newuserKey);
    memcpy(key, newuserKey, AES_KEY_SIZE);

    map<string, vector<int> > newPairs;
    newPairs = testCase->filePairs;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {
        vector<int>::iterator it=cur->second.begin();
        for(;it != cur->second.end();) {
            if(*it==ind){
                cur->second.erase(it);
                break;
            }
        }
    }

    vector<pair<prf_type,prf_type>> KeyValues;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {   //for each keyword 
        for (unsigned int j = 0; j < cur->second.size(); j++) {  //for each file id
            pair<prf_type,prf_type> keyval = share(cur->first, cur->second[j], user);
            KeyValues.push_back(keyval);
        }
    }

    server->update(KeyValues);
    if(ForFU){
        endSetup(user);
    }else{
        endSetup();
    }
    std::cout << "Unshare finished." << std::endl;
}

pair<prf_type,prf_type> Owner::share(string keyword, int ind, int userID) {
    if (accessList.count(userID) == 0) {
        accessList[userID] = set<int>();
    }
    accessList[userID].insert(ind);
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());

    int fileCnt = 0;
    if (omaps.count(userID) != 0) {
        std::array< uint8_t, ID_SIZE> mapkey;
        std::fill(mapkey.begin(), mapkey.end(), 0);
        std::copy(k_w.begin(), k_w.end(), mapkey.begin());
        Bid keyBid(mapkey);
        string value;
        if (setupMode) {
            if (setupPairs[userID].count(mapkey) == 0) {
                setupPairs[userID][mapkey] = "1";
            } else {
                setupPairs[userID][mapkey] = to_string(stoi(setupPairs[userID][mapkey]) + 1);
            }
            value = setupPairs[userID][mapkey];
        } else {
            omaps[userID]->CommunicationSize = 0;
            value = omaps[userID]->incrementFileCnt(keyBid);
            updateCommunicationSize += omaps[userID]->CommunicationSize;
        }
        fileCnt = stoi(value);
    } 

    prf_type addr, rnd;
    getAESRandomValue(k_w.data(), 0, fileCnt, addr.data(), keys[userID]);
    getAESRandomValue(k_w.data(), 1, fileCnt, rnd.data(), keys[userID]);

    prf_type val = bitwiseXOR(ind, rnd);
    if (!setupMode) {
        server->update(addr, val);
    }

    pair<prf_type,prf_type> keyval(addr,val);

    updateCommunicationSize += 2 * AES_KEY_SIZE;
    return keyval;
}

void Owner::unshare(int ind, OMAPBasedUser* user, const TC<int>* testCase){
    accessList[user->userID].clear();
    beginSetup();
    unsigned char newuserKey[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        newuserKey[i] = rand() % 256;
    }
    memcpy(user->key, newuserKey, AES_KEY_SIZE);
    registerUser(user->userID, newuserKey, testCase->K * 2, user->omapkey);
    
    

    map<string, vector<int> > newPairs;
    newPairs = testCase->filePairs;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {
        vector<int>::iterator it=cur->second.begin();
        for(;it != cur->second.end();) {
            if(*it==ind){
                cur->second.erase(it);
                break;
            }
        }
    }


    vector<pair<prf_type,prf_type>> KeyValues;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {   //for each keyword 
        for (unsigned int j = 0; j < cur->second.size(); j++) {  //for each file id
            pair<prf_type,prf_type> keyval = share(cur->first, cur->second[j], user->userID);
            KeyValues.push_back(keyval);
        }
    }

    server->update(KeyValues);

    endSetup();
    std::cout << "Unshare finished." << std::endl;
}

void Owner::getAESRandomValue(unsigned char* keyword, int op, int fileCnt, unsigned char* result, unsigned char* curkey) {
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    Utilities::encrypt((unsigned char*) keyword, AES_KEY_SIZE - 1, curkey, iv, result);
}

prf_type Owner::bitwiseXOR(int input1, prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    //result[4] = input2[4] ^ (op & 0xFF);
    for (int i = 4; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type Owner::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void Owner::registerUser(int curUserID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey) {//Key=128,means omapkey has 128 bytes
    keys[curUserID] = new unsigned char[AES_KEY_SIZE];
    memcpy(keys[curUserID], key, AES_KEY_SIZE);
    omaps[curUserID] = new OMAP(maxKeywordSize, omapkey, server, curUserID);
}

void Owner::registerUser(int curUserID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey, MusesOwnerRunner* ownerRunner) {
    keys[curUserID] = new unsigned char[AES_KEY_SIZE];
    memcpy(keys[curUserID], key, AES_KEY_SIZE);
    omaps[curUserID] = new OMAP(maxKeywordSize, omapkey, ownerRunner, curUserID);
}

void Owner::registerUser(int curUserID, unsigned char* key) {//for construction 1 and 2
    keys[curUserID] = new unsigned char[AES_KEY_SIZE]; //keys is a map to store all user keys
    memcpy(keys[curUserID], key, AES_KEY_SIZE);
    allFileCnt[curUserID] = map<prf_type, int>();
}

// void Owner::uploadMerkleRoot(int curUserID) {
//     if (setupMode) {
//         memcpy(server->valuesMerkleRoot, valuesMerkleRoot, SHA256_DIGEST_LENGTH);
//         memcpy(server->queuesMerkleRoot, queuesMerkleRoot, SHA256_DIGEST_LENGTH);
//         updateCommunicationSize += SHA256_DIGEST_LENGTH * 2;

//         if (omaps.count(curUserID) != 0) {
//             memcpy(server->omapsMerkleRoot[curUserID], omapsMerkleRoot[curUserID], SHA256_DIGEST_LENGTH);
//             updateCommunicationSize += SHA256_DIGEST_LENGTH;
//         }
//     } /*else {
//         string command = hyperLedgerOwnerUpdateTreeRootCommand;
//         Utilities::replace(command, "OWNERID", to_string(userID));
//         Utilities::replace(command, "OWNERID", to_string(userID));
//         Utilities::replace(command, "HYPERLEDGERKEY", hyperledgerKey);
//         Utilities::replace(command, "NEWROOT", Utilities::base64_encode((const char*) valuesMerkleRoot, SHA256_DIGEST_LENGTH) + "--" + Utilities::base64_encode((const char*) queuesMerkleRoot, SHA256_DIGEST_LENGTH));

//         (runner == NULL ? Utilities::executeCommand(command.c_str()) : runner->executeCommandOnFabric(command));
//         updateCommunicationSize += SHA256_DIGEST_LENGTH * 2;

//         if (omaps.count(curUserID) != 0) {
//             command = hyperLedgerOwnerUpdateTreeRootCommand;
//             Utilities::replace(command, "OWNERID", to_string(curUserID));
//             Utilities::replace(command, "OWNERID", to_string(curUserID));
//             Utilities::replace(command, "HYPERLEDGERKEY", blockchainsKey[curUserID]);
//             Utilities::replace(command, "NEWROOT", Utilities::base64_encode((const char*) omapsMerkleRoot[curUserID], SHA256_DIGEST_LENGTH));
//             (runner == NULL ? Utilities::executeCommand(command.c_str()) : runner->executeCommandOnFabric(command));
//             updateCommunicationSize += SHA256_DIGEST_LENGTH;
//         }
//     }*/
// }

bool Owner::isUserQueueBased(int userID) {
    return (omaps.count(userID) == 0);
}

void Owner::beginSetup() {
    setupMode = true;
}

void Owner::endSetup() {
    setupMode = false;
    for (auto item : omaps) {
        item.second->setupInsert(setupPairs[item.first]);
    }
}

void Owner::endSetup(QueueBasedUser* user) {
    setupMode = false;
    user->FileCnt.clear();
    map<prf_type, int>::iterator iter;
    for(iter = allFileCnt[user->userID].begin(); iter != allFileCnt[user->userID].end(); iter++){
        user->FileCnt[iter->first]=iter->second;
    }
    updateCommunicationSize += allFileCnt[user->userID].size()*(AES_KEY_SIZE+4);//4 is int size (4 bytes),update the user's Filecnt, k_w is AES_KEY_SIZE, cnt is 4
}
