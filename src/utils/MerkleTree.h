#ifndef MERKLETREE_H
#define MERKLETREE_H
#include "../utils/Utilities.h"
#include <stdio.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <array>

using namespace std;

class MerkleNode {
public:
    int id;
    MerkleNode* left = NULL;
    MerkleNode* right = NULL;
    MerkleNode* parent = NULL;
    unsigned char digest[SHA256_DIGEST_LENGTH] = {0};

    void computeHash();
    void setDigest(unsigned char* hashValue);
};

class MerkleTree {
private:
    map<int, MerkleNode*> leafs;
    MerkleNode* root = NULL;
    MerkleNode* buildMerkleTree(int size);
    void printTree(MerkleNode* root, int indent);
    void assignIDs(MerkleNode* root);

public:
    int size = 0;
    void addNode(unsigned char* hashValue, int pos);
    void getProof(vector<int> poses, map<int, unsigned char*>& proof);
    MerkleTree();
    virtual ~MerkleTree();
    void upgradeSize(int size);

};



#endif /* MERKLETREE_H */

