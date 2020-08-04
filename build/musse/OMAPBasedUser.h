#ifndef OMAPUSER_H
#define OMAPUSER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include "Server.h"
#include <iostream>
#include <sstream>
#include "musse/Server.h"
#include "utils/Utilities.h"
#include "OMAP.h"
using namespace std;

class MusesUserRunner;

class OMAPBasedUser {
    friend class Owner;
    friend class MusesOwnerRunner;
    friend class MusesUserRunner;

private:
    inline prf_type bitwiseXOR(int input1, int op, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int counter, unsigned char* result, unsigned char* curkey);
    Server* server = NULL;
    MusesUserRunner* runner = NULL;
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[AES_KEY_SIZE];
    // void downloadMerkleRoot();
    // string hyperLedgerGetRootCommand;

    // string importParticipant = "composer card import --file /tmp/CARDNAME.card --card CARDNAME@Muses";
    // string createClientCard = "composer identity issue -c admin@Muses -f /tmp/cidREST.card -u cidREST -a \"resource:com.huawei.Client#REST\"";
    // string createClientParticipant = "composer participant add -c admin@Muses -d '{\"$class\": \"com.huawei.Client\",\"clientId\": \"REST\"}'";
    // string checkExistanceOfRESTClient = "composer identity list -c admin@Muses | grep \"cidREST\"";
    // string createOwnerCard = "composer identity issue -c admin@Muses -f /tmp/oidOWNERID.card -u oidOWNERID -a \"resource:com.huawei.Owner#OWNERID\"";
    // string createOwnerParticipant = "composer participant add -c admin@Muses -d '{\"$class\": \"com.huawei.Owner\",\"ownerId\": \"OWNERID\", \"secret\":\"HYPERLEDGERKEY\"}'";
    // string hyperLedgerOwnerMerkleTreeConstructionCommand = "composer transaction submit -c oidOWNERID@Muses -d '{\"$class\": \"org.hyperledger.composer.system.AddAsset\", \"resources\": [  {   \"$class\": \"com.huawei.MerkleRoot\", \"rootId\": \"ROOTID\",   \"rootHash\": \"ROOTHASH\",   \"owner\": \"resource:com.huawei.Owner#OWNERID\"  } ], \"targetRegistry\": \"resource:org.hyperledger.composer.system.AssetRegistry#com.huawei.MerkleRoot\"}'";
    // string hyperLedgerOwnerUpdateTreeRootCommand = "composer transaction submit -c oidOWNERID@Muses -d '{\"$class\":\"com.huawei.ChangeRoot\",\"root\":\"resource:com.huawei.MerkleRoot#OWNERID\",  \"newHash\": \"NEWROOT\", \"secret\": \"HYPERLEDGERKEY\"}'";
    // bool useHyperLedgerFabric;
    //bool verifyMerkleProof(vector<prf_type> values, vector<int> valuesPoses, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot);
    // void uploadMerkleRoot();

public:
    int userID;
    int ownerID;
    bytes<Key> omapkey;
    // string hyperledgerKey;
    OMAP* omap;
    int searchCommunicationSize = 0;
    // unsigned char valuesMerkleRoot[SHA256_DIGEST_LENGTH];
    // unsigned char* omapMerkleRoot;
    OMAPBasedUser(Server* server, unsigned char* masterKey, int maxKeywordSize, int ownerID, MusesUserRunner* runner);
    int getFileCnt(prf_type input);
    void setFileCnt(prf_type input, int cnt);
    int incFileCnt(prf_type input);
    vector<int> search(string keyword);
    //vector<int> search(string keyword, map<int, unsigned char*>& proofs);
    prf_type searchRequest(string keyword, vector<prf_type>& tokens);
    void searchProcess(vector<prf_type> values, prf_type k_w, vector<int>& finalRes);
    virtual ~OMAPBasedUser();
};

#endif /* OMAPUSER_H */
