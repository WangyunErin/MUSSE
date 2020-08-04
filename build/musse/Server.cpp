#include "Server.h"
#include <iostream>
#include <sstream>
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <vector>
#include <algorithm>
#include "utils/Utilities.h"
#include <numeric>

Server::Server(int maxCounterSize) {
    maxCounter = maxCounterSize;
    keepsearching = 1;
}

Server::~Server() {
}

void Server::update(prf_type addr, prf_type val) {
    DictW[addr] = val;
}


void Server::update(vector<pair<prf_type,  prf_type> > KeyValues) {
    for (unsigned int i=0; i<KeyValues.size(); i++)
    {
        DictW[KeyValues[i].first] =KeyValues[i].second;
    }
    
    //unsigned char* hash = Utilities::getSHA256(val.data(), AES_KEY_SIZE);
    //valuesMerkleTree.addNode(hash, counter);

    // ResultWithProof result;
    // result.treeSize = valuesMerkleTree.size;
    // result.valuesPoses.push_back(counter);
    // result.values.push_back(val);
    // valuesMerkleTree.getProof(result.valuesPoses, result.proofs);
    // counter++;
    // return result;
}

prf_type Server::search(prf_type addr){
    prf_type val;
    prf_type notfound;
    memset(notfound.data(), 0, AES_KEY_SIZE);
    if (DictW.count(addr) == 0) {
        keepsearching=0;
        return notfound;
    }
    else{
        val = DictW[addr];
    }
    return val;
}

vector<prf_type> Server::search(vector<prf_type> KList) {
    vector<prf_type> result;
    for (unsigned int i = 0; i < KList.size(); i++) {
        if (DictW.count(KList[i]) != 0) {
            prf_type val = DictW[KList[i]];
            result.emplace_back(val);
        }
    }
    return result;
}

void Server::createRamStore(size_t num, int userID) {
    if (ramStores.count(userID) == 0) {
        RAMStore* store = new RAMStore(num);
        ramStores[userID] = store;
        // omapsMerkleRoot[userID] = new unsigned char[SHA256_DIGEST_LENGTH];
        // omapsMerkleTree[userID] = new MerkleTree();
        // omapsMerkleTree[userID]->upgradeSize(num);
    }
}

BlocksWithProof Server::writeInStore(vector<int> poses, vector<block> blocks, int userID) {
    BlocksWithProof res;
    for (unsigned int i = 0; i < poses.size(); i++) {
        ramStores[userID]->Write(poses[i], blocks[i]);
        unsigned char* hash = Utilities::getSHA256(blocks[i].data(), blocks[i].size());
        // omapsMerkleTree[userID]->addNode(hash, poses[i]);
    }
    // res.treeSize = omapsMerkleTree[userID]->size;
    res.values.insert(res.values.begin(), blocks.begin(), blocks.end());
    res.valuesPoses.insert(res.valuesPoses.begin(), poses.begin(), poses.end());
    // omapsMerkleTree[userID]->getProof(res.valuesPoses, res.proofs);
    return res;
}

BlocksWithProof Server::readStore(vector<int> poses, int userID) {
    BlocksWithProof res;
    for (auto pos : poses) {
        res.values.push_back(ramStores[userID]->Read(pos));
    }
    res.valuesPoses.insert(res.valuesPoses.begin(), poses.begin(), poses.end());
    // res.treeSize = omapsMerkleTree[userID]->size;
    // omapsMerkleTree[userID]->getProof(res.valuesPoses, res.proofs);
    return res;
}

// ResultWithProof Server::addToFileCntQueue(int userID, prf_type newItem) {
//     ResultWithProof res;
//     if (FileCntQueues.count(userID) == 0) {
//         FileCntQueues[userID] = vector<prf_type>();
//         FileCntQueues[userID].reserve(maxCounter);
//         queuesMerkleTree.upgradeSize(FileCntQueues.size() * maxCounter);
//         usersQueuePointer[userID] = 0;
//         int id = usersOrder.size();
//         usersOrder[userID] = id;
//     }
//     FileCntQueues[userID].push_back(newItem);
//     unsigned char* hash = Utilities::getSHA256(newItem.data(), AES_KEY_SIZE);
//     queuesMerkleTree.addNode(hash, usersOrder[userID] * maxCounter + FileCntQueues[userID].size() - 1);
//     res.treeSize = queuesMerkleTree.size;
//     res.values.push_back(newItem);
//     res.valuesPoses.push_back(usersOrder[userID] * maxCounter + FileCntQueues[userID].size() - 1);
//     queuesMerkleTree.getProof(res.valuesPoses, res.proofs);
//     return res;
// }

// ResultWithProof Server::getQueueValues(int userID) {
//     ResultWithProof res;
//     auto first = FileCntQueues[userID].cbegin() + usersQueuePointer[userID];
//     auto last = FileCntQueues[userID].cend();
//     res.values = vector<prf_type>(first, last);
//     res.valuesPoses = vector<int>(FileCntQueues[userID].size() - usersQueuePointer[userID]);
//     iota(res.valuesPoses.begin(), res.valuesPoses.end(), usersOrder[userID] * maxCounter + usersQueuePointer[userID]);
//     queuesMerkleTree.getProof(res.valuesPoses, res.proofs);
//     res.treeSize = queuesMerkleTree.size;
//     usersQueuePointer[userID] = FileCntQueues[userID].size();
//     return res;
// }

// ResultWithProof Server::getQueueValues(int userID, int remain) {
//     ResultWithProof res;
//     auto first = FileCntQueues[userID].cbegin() + usersQueuePointer[userID];
//     auto last = FileCntQueues[userID].cend() - remain;
//     res.values = vector<prf_type>(first, last);
//     res.valuesPoses = vector<int>(FileCntQueues[userID].size() - usersQueuePointer[userID] - remain);
//     iota(res.valuesPoses.begin(), res.valuesPoses.end(), usersOrder[userID] * maxCounter + usersQueuePointer[userID]);
//     queuesMerkleTree.getProof(res.valuesPoses, res.proofs);
//     res.treeSize = queuesMerkleTree.size;
//     usersQueuePointer[userID] = FileCntQueues[userID].size() - remain;
//     return res;
// }

void Server::uploadStash(vector< pair<int, unsigned char*> > stash, int userID) {
    stashes[userID] = stash;
}

vector< pair<int, unsigned char*> > Server::downloadStash(int userID) {
    vector< pair<int, unsigned char*> > result = stashes[userID];
    stashes.erase(userID);
    return result;
}

pair<Bid, int> Server::getOMAPRoot(int userID) {
    return omaproots[userID];
}

void Server::setOMAPRoots(Bid bid, int pos, int userID) {
    omaproots[userID] = pair<Bid, int>(bid, pos);
}


