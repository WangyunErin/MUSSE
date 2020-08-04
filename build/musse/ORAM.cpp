#include "ORAM.hpp"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <random>
#include <cmath>
#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <stdlib.h>
#include <vector>
#include "MUSSEUserRunner.h"
#include "OwnerRunner.h"

ORAM::ORAM(int maxSize, bytes<Key> oram_key, Server* server, int userID)
: key(oram_key), dis(0, (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    maxOfRandom = (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2;
    AES::Setup();
    this->userID = userID;
    this->server = server;
    // this->merkleRoot = merkleRoot;
    depth = (int) floor(log2(maxSize / Z));
    bucketCount = (int) pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = (size_t) (Z * (pow(2, depth + 1) - 1));
    maxHeightOfAVLTree = (int) floor(log2(blockCount)) + 1;
    clen_size = AES::GetCiphertextLength((int) (blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    server->createRamStore(blockCount, userID);
    vector<Bucket> buks;
    vector<int> indexes;
    for (int i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        buks.push_back(bucket);
        indexes.push_back(i);
    }
    WriteBuckets(indexes, buks);
}

ORAM::ORAM(int maxSize, bytes<Key> oram_key, MusesUserRunner* runner, int userID)
: key(oram_key), dis(0, (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    maxOfRandom = (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2;
    AES::Setup();
    this->userID = userID;
    this->runner = runner;
    // this->merkleRoot = merkleRoot;
    depth = (int) floor(log2(maxSize / Z));
    bucketCount = (int) pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = (size_t) (Z * (pow(2, depth + 1) - 1));
    maxHeightOfAVLTree = (int) floor(log2(blockCount)) + 1;
    clen_size = AES::GetCiphertextLength((int) (blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    runner->createRamStore(blockCount, clen_size, userID);
    vector<Bucket> buks;
    vector<int> indexes;
    for (int i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        buks.push_back(bucket);
        indexes.push_back(i);
    }
    WriteBuckets(indexes, buks);
}

ORAM::ORAM(int maxSize, bytes<Key> oram_key, MusesOwnerRunner* runner, int userID)
: key(oram_key), dis(0, (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2) {
    maxOfRandom = (int) (pow(2, floor(log2(maxSize / Z)) + 1) - 1) / 2;
    AES::Setup();
    this->userID = userID;
    this->ownerrunner = runner;
    // this->merkleRoot = merkleRoot;
    depth = (int) floor(log2(maxSize / Z));
    bucketCount = (int) pow(2, depth + 1) - 1;
    blockSize = sizeof (Node); // B
    size_t blockCount = (size_t) (Z * (pow(2, depth + 1) - 1));
    maxHeightOfAVLTree = (int) floor(log2(blockCount)) + 1;
    clen_size = AES::GetCiphertextLength((int) (blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    ownerrunner->createRamStore(blockCount, clen_size, userID);
    vector<Bucket> buks;
    vector<int> indexes;
    for (int i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        buks.push_back(bucket);
        indexes.push_back(i);
    }
    WriteBuckets(indexes, buks);
}

ORAM::~ORAM() {
    AES::Cleanup();
}

// Fetches the array index a bucket
// that lise on a specific path

int ORAM::GetNodeOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block ORAM::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    assert(buffer.size() == Z * (blockSize));

    return buffer;
}

Bucket ORAM::DeserialiseBucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &curBlock = bucket[z];

        curBlock.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Node* node = convertBlockToNode(curBlock.data);
        curBlock.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

vector<Bucket> ORAM::ReadBuckets(vector<int> indexes) {
    totalRead += indexes.size();
    vector<Bucket> res;
     BlocksWithProof response = (server == NULL ? runner == NULL ? ownerrunner->readStore(indexes, userID) : runner->readStore(indexes, userID) : server->readStore(indexes, userID));
    // if (!verifyMerkleProof(response.values, response.valuesPoses, response.proofs, response.treeSize, merkleRoot)) {
    //     cout << "The server has modified the ORAM!" << endl;
    //     exit(0);
    // }
    for (auto b : response.values) {
        block buffer = AES::Decrypt(key, b, clen_size);
        Bucket bucket = DeserialiseBucket(buffer);
        res.push_back(bucket);
        CommunicationSize += clen_size;
        CommunicationSize += 4;
    }
    CommunicationSize += indexes.size()*4;
    CommunicationSize += indexes.size() * clen_size;
    CommunicationSize += 4;
    // CommunicationSize += response.proofs.size()*(SHA256_DIGEST_LENGTH + 4);
    // CommunicationSize += response.valuesPoses.size()*4;
    return res;
}

void ORAM::WriteBuckets(vector<int> indexes, vector<Bucket> buckets) {

    vector<block> ciphertexts;
    for (unsigned int i = 0; i < buckets.size(); i++) {
        totalWrite++;
        block b = SerialiseBucket(buckets[i]);
        block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
        ciphertexts.push_back(ciphertext);
    }
    if (server == NULL) {
        if (runner != NULL) {
            BlocksWithProof res = runner->writeInStore(indexes, ciphertexts, userID);
            // updateMerkleProof(ciphertexts[0], res.valuesPoses[0], res.proofs, res.treeSize, merkleRoot);
            CommunicationSize += indexes.size()*4;
            CommunicationSize += ciphertexts.size() * clen_size;
            CommunicationSize += 4;
            // CommunicationSize += (int) log2(res.treeSize)*(SHA256_DIGEST_LENGTH + 4);
            // CommunicationSize += 4;
        } else {
            BlocksWithProof res = ownerrunner->writeInStore(indexes, ciphertexts, userID);
            // updateMerkleProof(ciphertexts[0], res.valuesPoses[0], res.proofs, res.treeSize, merkleRoot);
            CommunicationSize += indexes.size()*4;
            CommunicationSize += ciphertexts.size() * clen_size;
            CommunicationSize += 4;
            // CommunicationSize += (int) log2(res.treeSize)*(SHA256_DIGEST_LENGTH + 4);
            // CommunicationSize += 4;
        }
    } else {
        BlocksWithProof res = server->writeInStore(indexes, ciphertexts, userID);
        // updateMerkleProof(ciphertexts[0], res.valuesPoses[0], res.proofs, res.treeSize, merkleRoot);
        CommunicationSize += indexes.size()*4;
        CommunicationSize += ciphertexts.size() * clen_size;
        CommunicationSize += 4;
        // CommunicationSize += (int) log2(res.treeSize)*(SHA256_DIGEST_LENGTH + 4);
        // CommunicationSize += 4;
    }

}

// Fetches blocks along a path, adding them to the cache

void ORAM::FetchPath(int leaf) {
    readCnt++;
    vector<int> nodes;
    for (int d = 0; d <= depth; d++) {
        int node = GetNodeOnPath(leaf, d);

        if (find(readviewmap.begin(), readviewmap.end(), node) != readviewmap.end()) {
            continue;
        } else {
            readviewmap.push_back(node);
        }
        nodes.push_back(node);
    }
    vector<Bucket> buckets = ReadBuckets(nodes);
    for (unsigned int i = 0; i < buckets.size(); i++) {

        Bucket bucket = buckets[i];

        for (int z = 0; z < Z; z++) {
            Block &curBlock = bucket[z];

            if (curBlock.id != 0) { // It isn't a dummy block   
                Node* n = convertBlockToNode(curBlock.data);
                if (cache.count(curBlock.id) == 0) {
                    cache.insert(make_pair(curBlock.id, n));
                } else {
                    delete n;
                }
            }
        }
    }
}

// Gets a list of blocks on the cache which can be placed at a specific point

std::vector<Bid> ORAM::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<Bid> validBlocks;

    int node = GetNodeOnPath(x, curDepth);
    for (auto b : cache) {
        Bid bid = b.first;
        if (b.second != NULL && GetNodeOnPath(b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

// Greedily writes blocks from the cache to the tree, pushing blocks as deep into the tree as possible

void ORAM::WritePath(int leaf, int d, vector<Bucket>& buckets, vector<int>& indexes) {
    // Find blocks that can be on this bucket
    int node = GetNodeOnPath(leaf, d);
    if (find(writeviewmap.begin(), writeviewmap.end(), node) == writeviewmap.end()) {

        auto validBlocks = GetIntersectingBlocks(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &curBlock = bucket[z];
            curBlock.id = validBlocks[z];
            Node* curnode = cache[curBlock.id];
            curBlock.data = convertNodeToBlock(curnode);
            delete curnode;
            cache.erase(curBlock.id);
        }
        // Fill any empty spaces with dummy blocks
        for (long unsigned int z = validBlocks.size(); z < Z; z++) {
            Block &curBlock = bucket[z];
            curBlock.id = 0;
            curBlock.data.resize(blockSize, 0);
        }

        // Write bucket to tree
        writeviewmap.push_back(node);
        buckets.push_back(bucket);
        indexes.push_back(node);
    }
}

// Gets the data of a block in the cache

Node* ORAM::ReadData(Bid bid) {
    if (cache.find(bid) == cache.end()) {
        return NULL;
    }
    return cache[bid];
}

// Updates the data of a block in the cache

void ORAM::WriteData(Bid bid, Node* node) {
    cache[bid] = node;
}

// Fetches a block, allowing you to read and write in a block

void ORAM::Access(Bid bid, Node*& node, int lastLeaf, int newLeaf) {
    FetchPath(lastLeaf);
    node = ReadData(bid);
    if (node != NULL) {
        node->pos = newLeaf;
        if (cache.count(bid) != 0) {
            cache.erase(bid);
        }
        cache[bid] = node;
        if (find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
            leafList.push_back(lastLeaf);
        }
    }
}

void ORAM::Access(Bid bid, Node*& node) {
    if (!batchWrite) {
        FetchPath(node->pos);
    }
    WriteData(bid, node);
    if (find(leafList.begin(), leafList.end(), node->pos) == leafList.end()) {
        leafList.push_back(node->pos);
    }
}

Node* ORAM::ReadNodeFromCache(Bid bid) {
    if (bid == 0) {
        throw runtime_error("Node id is not set");
    }
    if (cache.count(bid) == 0) {
        throw runtime_error("Node not found in the cache");
    } else {
        Node* node = cache[bid];
        return node;
    }
}

Node* ORAM::ReadNode(Bid bid, int lastLeaf, int newLeaf) {
    if (bid == 0) {
        return NULL;
    }
    if (cache.count(bid) == 0 || find(leafList.begin(), leafList.end(), lastLeaf) == leafList.end()) {
        Node* node;
        Access(bid, node, lastLeaf, newLeaf);
        if (node != NULL) {
            modified.insert(bid);
        }
        return node;
    } else {
        modified.insert(bid);
        Node* node = cache[bid];
        node->pos = newLeaf;
        return node;
    }
}

int ORAM::WriteNode(Bid bid, Node* node) {
    if (bid == 0) {
        throw runtime_error("Node id is not set");
    }
    if (cache.count(bid) == 0) {
        modified.insert(bid);
        Access(bid, node);
        return node->pos;
    } else {
        modified.insert(bid);
        return node->pos;
    }
}

Node* ORAM::convertBlockToNode(block b) {
    Node* node = new Node();
    std::array<byte_t, sizeof (Node) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Node), arr.begin());
    from_bytes(arr, *node);
    return node;
}

block ORAM::convertNodeToBlock(Node* node) {
    std::array<byte_t, sizeof (Node) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void ORAM::finilize(bool find, Bid& rootKey, int& rootPos) {
    //fake read for padding     
    if (!batchWrite) {
        if (find) {
            for (unsigned int i = readCnt; i < depth * 1.45; i++) {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) {
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
            }
        } else {
            for (int i = readCnt; i < 4.35 * depth; i++) {
                int rnd = RandomPath();
                if (std::find(leafList.begin(), leafList.end(), rnd) == leafList.end()) {
                    leafList.push_back(rnd);
                }
                FetchPath(rnd);
            }
        }
    }

    //updating the binary tree positions
    /**
     *  For general case, the following code extracts the maximum height in the stash. But,
     *  we assume that the use does not insert more than OMAP capacity to increase the performance
      
      
        int maxHeight = 1;
        for (auto t : cache) {
            if (t.second != NULL && t.second->height > maxHeight) {
                maxHeight = t.second->height;
            }
        }
     */

    for (int i = 1; i <= maxHeightOfAVLTree; i++) {
        for (auto t : cache) {
            if (t.second != NULL && t.second->height == i) {
                Node* tmp = t.second;
                if (modified.count(tmp->key)) {
                    tmp->pos = RandomPath();
                }
                if (tmp->leftID != 0 && cache.count(tmp->leftID) > 0) {
                    tmp->leftPos = cache[tmp->leftID]->pos;
                }
                if (tmp->rightID != 0 && cache.count(tmp->rightID) > 0) {
                    tmp->rightPos = cache[tmp->rightID]->pos;
                }
            }
        }
    }
    if (cache[rootKey] != NULL)
        rootPos = cache[rootKey]->pos;
    int cnt = 0;
    vector<Bucket> buks;
    vector<int> indexes;
    for (int d = depth; d >= 0; d--) {
        for (unsigned int i = 0; i < leafList.size(); i++) {
            cnt++;
            WritePath(leafList[i], d, buks, indexes);
            if (cnt % 1000 == 0 && batchWrite) {
                cout << "OMAP:" << cnt << "/" << (depth + 1) * leafList.size() << " inserted" << endl;
            }
        }
    }
    WriteBuckets(indexes, buks);
    leafList.clear();
    modified.clear();
    uploadStash();
}

void ORAM::start(bool isBatchWrite) {
    this->batchWrite = isBatchWrite;
    writeviewmap.clear();
    readviewmap.clear();
    readCnt = 0;
    downloadStash();
}

int ORAM::RandomPath() {
    int val = dis(mt);
    return val;
}

void ORAM::setupInsert(vector<Node*> nodes) {
    downloadStash();
    sort(nodes.begin(), nodes.end(), [ ](const Node* lhs, const Node * rhs) {
        return lhs->pos < rhs->pos;
    });
    int curPos = 0;
    if (nodes.size() > 0) {
        curPos = nodes[0]->pos;
    }
    map<int, Bucket> buckets;
    map<int, int> bucketsCnt;
    int cnt = 0;
    unsigned int i = 0;
    bool cannotInsert = false;
    while (i < nodes.size()) {
        cnt++;
        if (cnt % 1000 == 0) {
            cout << "i:" << i << "/" << nodes.size() << endl;
        }
        for (int d = depth; d >= 0 && i < nodes.size() && curPos == nodes[i]->pos; d--) {
            int nodeIndex = GetNodeOnPath(curPos, d);
            Bucket bucket;
            if (bucketsCnt.count(nodeIndex) == 0) {
                bucketsCnt[nodeIndex] = 0;
                for (int z = 0; z < Z; z++) {
                    if (i < nodes.size() && nodes[i]->pos == curPos) {
                        Block &curBlock = bucket[z];
                        curBlock.id = nodes[i]->key;
                        curBlock.data = convertNodeToBlock(nodes[i]);
                        delete nodes[i];
                        bucketsCnt[nodeIndex]++;
                        i++;
                    }
                }
                buckets[nodeIndex] = bucket;
            } else {
                if (bucketsCnt[nodeIndex] < Z) {
                    bucket = buckets[nodeIndex];
                    for (int z = bucketsCnt[nodeIndex]; z < Z; z++) {
                        if (i < nodes.size() && nodes[i]->pos == curPos) {
                            Block &curBlock = bucket[z];
                            curBlock.id = nodes[i]->key;
                            curBlock.data = convertNodeToBlock(nodes[i]);
                            delete nodes[i];
                            bucketsCnt[nodeIndex]++;
                            i++;
                        }
                    }
                    buckets[nodeIndex] = bucket;
                } else {
                    cannotInsert = true;
                }
            }

        }

        if (i < nodes.size()) {
            if (cannotInsert) {
                cache[nodes[i]->key] = nodes[i];
                i++;
                cannotInsert = false;
            }
            if (i < nodes.size()) {
                curPos = nodes[i]->pos;
            }
        }
    }

    vector<int> indexes;
    vector<Bucket> finalbuckets;
    for (auto buk : buckets) {
        if (bucketsCnt[buk.first] == Z) {
            indexes.push_back(buk.first);
            finalbuckets.push_back(buk.second);
        } else {
            for (long unsigned int z = bucketsCnt[buk.first]; z < Z; z++) {
                Block &curBlock = buk.second[z];
                curBlock.id = 0;
                curBlock.data.resize(blockSize, 0);
            }
            indexes.push_back(buk.first);
            finalbuckets.push_back(buk.second);
        }
    }

    WriteBuckets(indexes, finalbuckets);
    for (; i < nodes.size(); i++) {
        cache[nodes[i]->key] = nodes[i];
    }
    uploadStash();
}

void ORAM::downloadStash() {
    unsigned char iv[AES_KEY_SIZE];
    unsigned char ckey[AES_KEY_SIZE];
    memset(iv, 0x00, AES_KEY_SIZE);
    memset(ckey, 0x00, AES_KEY_SIZE);
    vector< pair<int, unsigned char*> > stashItems;
    if (server != NULL) {
        stashItems = server->downloadStash(userID);
    } else if (runner != NULL) {
        stashItems = runner->downloadStash(userID);
    } else {
        stashItems = ownerrunner->downloadStash(userID);
    }
    if (stashItems.size() != 0) {
        for (int i = 0; i < 90; i++) {
            unsigned char* nodeData = new unsigned char[sizeof (Node) + 1];
            Utilities::decrypt((unsigned char*) stashItems[i].second, stashItems[i].first, ckey, iv, (unsigned char*) nodeData);
            Node* node = (Node*) nodeData;
            if (node->key.getValue() != 0) {
                cache[node->key] = node;
            }
            delete stashItems[i].second;
        }
    }
    CommunicationSize += 90 * sizeof (Node);
}

void ORAM::uploadStash() {
    vector< pair<int, unsigned char*> > stashItems;
    unsigned char iv[AES_KEY_SIZE];
    unsigned char ckey[AES_KEY_SIZE];
    memset(iv, 0x00, AES_KEY_SIZE);
    memset(ckey, 0x00, AES_KEY_SIZE);
    int i = 0;
    for (auto item : cache) {
        i++;
        unsigned char* cipher = new unsigned char[sizeof (Node) + 20];
        int size = Utilities::encrypt((unsigned char*) item.second, sizeof (Node), ckey, iv, cipher);
        stashItems.push_back(pair<int, unsigned char*> (size, cipher));
        delete item.second;
    }

    for (; i < 90; i++) {
        Node* dummy = new Node();
        dummy->key.setValue(0);
        unsigned char* cipher = new unsigned char[sizeof (Node) + 20];
        int size = Utilities::encrypt((unsigned char*) dummy, sizeof (Node), ckey, iv, cipher);
        stashItems.push_back(pair<int, unsigned char*> (size, cipher));
        delete dummy;
    }
    if (server != NULL) {
        server->uploadStash(stashItems, userID);
    } else if (runner != NULL) {
        runner->uploadStash(stashItems, userID);
        for (i = 0; i < 90; i++) {
            delete stashItems[i].second;
        }
    } else {
        ownerrunner->uploadStash(stashItems, userID);
        for (i = 0; i < 90; i++) {
            delete stashItems[i].second;
        }
    }

    cache.clear();
    CommunicationSize += 90 * sizeof (Node);
}

// bool ORAM::verifyMerkleProof(vector<block> values, vector<int> valuesPoses, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot) {
//     map<int, unsigned char*> verifiedNodes;

//     for (unsigned int i = 0; i < values.size(); i++) {
//         block decodedString = values[i];


//         unsigned char token[SHA256_DIGEST_LENGTH];
//         memcpy((char*) token, (char*) Utilities::getSHA256(decodedString.data(), decodedString.size()), SHA256_DIGEST_LENGTH);
//         valuesPoses[i] += treeSize;

//         while (valuesPoses[i] > 1) {
//             unsigned char* concat = new unsigned char[SHA256_DIGEST_LENGTH * 2];
//             if (valuesPoses[i] % 2 == 0) {
//                 //left child
//                 memcpy((char*) concat + SHA256_DIGEST_LENGTH, (char*) proofs[valuesPoses[i] + 1], SHA256_DIGEST_LENGTH);
//                 memcpy((char*) concat, (char*) token, SHA256_DIGEST_LENGTH);
//                 memcpy((char*) token, (char*) Utilities::getSHA256(concat, SHA256_DIGEST_LENGTH * 2), SHA256_DIGEST_LENGTH);
//                 valuesPoses[i] = valuesPoses[i] / 2;
//                 if (verifiedNodes.count(valuesPoses[i]) == 0) {
//                     verifiedNodes[valuesPoses[i]] = concat;
//                 } else {
//                     break;
//                 }
//             } else {
//                 //right child
//                 memcpy((char*) concat, (char*) proofs[valuesPoses[i] - 1], SHA256_DIGEST_LENGTH);
//                 memcpy((char*) concat + SHA256_DIGEST_LENGTH, (char*) token, SHA256_DIGEST_LENGTH);
//                 memcpy((char*) token, (char*) Utilities::getSHA256(concat, SHA256_DIGEST_LENGTH * 2), SHA256_DIGEST_LENGTH);
//                 valuesPoses[i] = (valuesPoses[i] - 1) / 2;
//                 if (verifiedNodes.count(valuesPoses[i]) == 0) {
//                     verifiedNodes[valuesPoses[i]] = concat;
//                 } else {
//                     break;
//                 }
//             }
//         }

//         if (valuesPoses[i] == 1 && memcmp(token, localRoot, SHA256_DIGEST_LENGTH) != 0) {
//             return false;
//         }
//     }
//     return true;
// }

// void ORAM::updateMerkleProof(block val, int pos, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot) {
//     pos += treeSize;
//     unsigned char token[SHA256_DIGEST_LENGTH];
//     memcpy((char*) token, (char*) Utilities::getSHA256(val.data(), val.size()), SHA256_DIGEST_LENGTH);
//     while (pos > 1) {
//         unsigned char concat[SHA256_DIGEST_LENGTH * 2];
//         if (pos % 2 == 0) {
//             //left child
//             memcpy((char*) concat + SHA256_DIGEST_LENGTH, (char*) proofs[pos + 1], SHA256_DIGEST_LENGTH);
//             memcpy((char*) concat, (char*) token, SHA256_DIGEST_LENGTH);
//             memcpy((char*) token, (char*) Utilities::getSHA256(concat, SHA256_DIGEST_LENGTH * 2), SHA256_DIGEST_LENGTH);
//             pos = pos / 2;
//         } else {
//             //right child
//             memcpy((char*) concat, (char*) proofs[pos - 1], SHA256_DIGEST_LENGTH);
//             memcpy((char*) concat + SHA256_DIGEST_LENGTH, (char*) token, SHA256_DIGEST_LENGTH);
//             memcpy((char*) token, (char*) Utilities::getSHA256(concat, SHA256_DIGEST_LENGTH * 2), SHA256_DIGEST_LENGTH);
//             pos = (pos - 1) / 2;
//         }
//     }
//     memcpy(localRoot, token, SHA256_DIGEST_LENGTH);
// }