#ifndef ORAM_H
#define ORAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include "RAMStore.hpp"
#include <bits/stdc++.h>
#include "Bid.h"
#include "Server.h"


class MusesUserRunner;
class MusesOwnerRunner;

using namespace std;

class Node {
public:

    Node() {
    };

    ~Node() {
    };

    Bid key;
    //    std::array< byte_t, 16> value;
    int value;
    int pos;
    Bid leftID;
    int leftPos;
    Bid rightID;
    int rightPos;
    int height;
};

struct Block {
    Bid id;
    block data;
};

using Bucket = std::array<Block, Z>;

class ORAM {
private:
    int depth;
    size_t blockSize;
    unordered_map<Bid, Node*, BidHasher> cache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<Bid> modified;
    int readCnt = 0;
    bytes<Key> key;
    size_t plaintext_size;
    int bucketCount;
    size_t clen_size;
    bool batchWrite = false;
    int maxOfRandom;
    int maxHeightOfAVLTree;
    int userID;
    Server* server = NULL;
    MusesUserRunner* runner = NULL;
    MusesOwnerRunner* ownerrunner = NULL;
    //unsigned char* merkleRoot;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetNodeOnPath(int leaf, int depth);
    vector<Bid> GetIntersectingBlocks(int x, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level, vector<Bucket>& buckets, vector<int>& indexes);

    Node* ReadData(Bid bid);
    void WriteData(Bid bid, Node* b);

    block SerialiseBucket(Bucket bucket);
    Bucket DeserialiseBucket(block buffer);

    vector<Bucket> ReadBuckets(vector<int> indexes);
    void WriteBuckets(vector<int> indexes, vector<Bucket> buckets);
    void Access(Bid bid, Node*& node, int lastLeaf, int newLeaf);
    void Access(Bid bid, Node*& node);



    Node* convertBlockToNode(block b);
    block convertNodeToBlock(Node* node);
    void downloadStash();
    void uploadStash();
    // bool verifyMerkleProof(vector<block> values, vector<int> valuesPoses, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot);
    // void updateMerkleProof(block val, int pos, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot);

public:
    ORAM(int maxSize, bytes<Key> key, Server* server, int userID);
    ORAM(int maxSize, bytes<Key> key, MusesUserRunner* runner, int userID);
    ORAM(int maxSize, bytes<Key> key, MusesOwnerRunner* runner, int userID);
    ~ORAM();
    int totalRead = 0, totalWrite = 0;
    int CommunicationSize;

    Node* ReadNode(Bid bid, int lastLeaf, int newLeaf);
    Node* ReadNodeFromCache(Bid bid);
    int WriteNode(Bid bid, Node* n);
    void start(bool batchWrite);
    void finilize(bool find, Bid& rootKey, int& rootPos);
    void setupInsert(vector<Node*> nodes);

};

#endif
