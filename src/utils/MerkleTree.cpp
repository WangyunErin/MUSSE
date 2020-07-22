#include "MerkleTree.h"
#include <bitset>
#include <cmath>

MerkleTree::MerkleTree() {
}

MerkleTree::~MerkleTree() {
}

void MerkleTree::addNode(unsigned char* hashValue, int pos) {
    if (size == pos) {
        MerkleNode* rightTree = buildMerkleTree(size);
        MerkleNode* leftTree = root;
        root = new MerkleNode();
        root->left = leftTree;
        if (leftTree != NULL) {
            leftTree->parent = root;
        }
        root->right = rightTree;
        if (rightTree != NULL) {
            rightTree->parent = root;
        }
        root->computeHash();
        size = (size == 0 ? 1 : (size * 2));
        root->id = 1;
        assignIDs(root);
    }

    MerkleNode* curNode = root;
    int bitsize = log2(size);
    string posStr = bitset <32>(pos).to_string();
    posStr = posStr.substr(posStr.length() - bitsize, bitsize);

    int index = 0;
    while (curNode->left != NULL) {
        if (posStr[index] == '0') {
            curNode = curNode->left;
        } else {
            curNode = curNode->right;
        }
        index++;
    }
    curNode->setDigest(hashValue);

    while (curNode != root) {
        curNode = curNode->parent;
        curNode->computeHash();
    }
    //    printTree(root, 0);
}

void MerkleTree::getProof(vector<int> poses, map<int, unsigned char*>& proofs) {
    for (unsigned int i = 0; i < poses.size(); i++) {
        int pos = poses[i];
        MerkleNode* curNode = leafs[pos];
        int bitsize = log2(size);
        string posStr = bitset <32>(pos).to_string();
        posStr = posStr.substr(posStr.length() - bitsize, bitsize);
        int index = posStr.length() - 1;
        while (curNode != root) {
            if (posStr[index] == '0') {
                if (proofs.count(curNode->parent->right->id) == 0) {
                    proofs[curNode->parent->right->id] = curNode->parent->right->digest;
                    curNode = curNode->parent;
                } else {
                    break;
                }
            } else {
                if (proofs.count(curNode->parent->left->id) == 0) {
                    proofs[curNode->parent->left->id] = curNode->parent->left->digest;
                    curNode = curNode->parent;
                } else {
                    break;
                }

            }
            index--;
        }
    }
}

MerkleNode* MerkleTree::buildMerkleTree(int size) {
    vector<MerkleNode*> nodes;
    for (int i = 0; i < size; i++) {
        MerkleNode* node = new MerkleNode();
        nodes.push_back(node);
    }

    while (size > 1) {
        size = size / 2;
        for (int i = 0; i < size; i++) {
            MerkleNode* node = new MerkleNode();
            node->left = nodes[i * 2];
            nodes[i * 2]->parent = node;
            node->right = nodes[i * 2 + 1];
            nodes[i * 2 + 1]->parent = node;
            nodes.push_back(node);
        }
        nodes.erase(nodes.begin(), nodes.begin() + (size * 2));
    }
    return nodes.size() > 0 ? nodes[0] : NULL;
}

void MerkleTree::printTree(MerkleNode* root, int indent) {
    if (root != NULL) {
        if (root->left != NULL) {
            printTree(root->left, indent + 4);
        }

        if (indent > 0) {
            for (int i = 0; i < indent; i++) {
                printf(" ");
            }
        }
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02X", root->digest[i]);
        }
        printf("\n");

        if (root->right != NULL) {
            printTree(root->right, indent + 4);
        }
    }

}

void MerkleTree::assignIDs(MerkleNode* root) {
    if (root->left != NULL) {
        root->left->id = 2 * root->id;
        assignIDs(root->left);
    } else {
        leafs[root->id - size] = root;
    }
    if (root->right != NULL) {
        root->right->id = 2 * root->id + 1;
        assignIDs(root->right);
    } else {
        leafs[root->id - size] = root;
    }
}

void MerkleTree::upgradeSize(int newSize) {
    while (size < newSize) {
        MerkleNode* rightTree = buildMerkleTree(size);
        MerkleNode* leftTree = root;
        root = new MerkleNode();
        root->left = leftTree;
        if (leftTree != NULL) {
            leftTree->parent = root;
        }
        root->right = rightTree;
        if (rightTree != NULL) {
            rightTree->parent = root;
        }
        root->computeHash();
        size = (size == 0 ? 1 : (size * 2));
        root->id = 1;
        assignIDs(root);
    };
}

void MerkleNode::computeHash() {
    if (left != NULL && right != NULL) {
        unsigned char concat[SHA256_DIGEST_LENGTH * 2];
        memcpy((char*) concat, (char*) left->digest, SHA256_DIGEST_LENGTH);
        memcpy((char*) concat + SHA256_DIGEST_LENGTH, (char*) right->digest, SHA256_DIGEST_LENGTH);
        memcpy((char*) digest, (char*) Utilities::getSHA256(concat, SHA256_DIGEST_LENGTH * 2), SHA256_DIGEST_LENGTH);
    }
}

void MerkleNode::setDigest(unsigned char* input) {
    memcpy((char*) digest, (char*) input, SHA256_DIGEST_LENGTH);
}
