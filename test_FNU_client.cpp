#include "src/musse/Owner.h"
#include "src/utils/Utilities.h"
#include "src/musse/Owner.h"
#include "src/musse/OMAPBasedUser.h"
#include "musse/OMAPBasedUser.h"
#include "musse/OwnerRunner.h"
#include "musse/MUSSEUserRunner.h"
using namespace std;

int main(int, char**) {
    TC<int> testCase;
    uint keywordLength = 16;
    string serverAdr = "18.221.85.245:4241";
    Utilities::readConfigFile("config.txt", testCase);
    Utilities::generateTestCases(testCase, keywordLength, 14);

    unsigned char masterKey[AES_KEY_SIZE], user1Key[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        masterKey[i] = rand() % 256;
        user1Key[i] = rand() % 256;
    }
    cout << "Master Key and Users' Keys Generated" << endl;

    MusesOwnerRunner client(serverAdr,testCase.N, masterKey);
    MusesUserRunner userRunner(serverAdr);
    OMAPBasedUser user(NULL, user1Key, testCase.K * 2, client.getOwnerID(), &userRunner);

    client.registerUser(user.userID, user1Key, testCase.K * 2, user.omapkey);

    cout << "Start of FNU" << endl;
    int cnt = 0;
    client.beginSetup();
    //Inserting the test cases (keyword,file) pairs
    for (auto cur = testCase.filePairs.begin(); cur != testCase.filePairs.end(); cur++) {
        for (unsigned int j = 0; j < cur->second.size(); j++) {
            client.share(cur->first, cur->second[j], user.userID);
            cnt++;
            if (cnt % 10000 == 0) {
                cout << "Initial Insertion:" << cnt << "/" << to_string(testCase.N) << endl;
            }
        }
    }
    client.endSetup();
    double time = 0;
    cnt = 0;
    for (uint j = 0; j < testCase.Qs.size(); j++) {
        cout << "---------------------" << endl;
        cout << "Result of Operations for DB Size " << testCase.N << endl;
        auto item = testCase.filePairs[testCase.testKeywords[j]];

        //measuring search and update execution times
        // cout << "Search for Keyword With " << testCase.Qs[j] << " Results " << endl;
        // user.searchCommunicationSize = 0;
        // for (int z = 0; z < 10; z++) {
        //     Utilities::startTimer(500);
        //     vector<int> res = userRunner.search(testCase.testKeywords[j], &user);
        //     time = Utilities::stopTimer(500);
        //     cout << "Search Computation Time (microseconds):" << time << endl;
        //     cout << "Search Communication Size (Bytes):" << user.searchCommunicationSize << endl;
        //     cout << "Number of return item:" << res.size() << endl;
        // }
        // Utilities::startTimer(500);
        client.unshare(item[0], &user, &testCase);
        // time = Utilities::stopTimer(500);
        // cout << "Unshare Time:" << time << endl;
        for (int z = 0; z < 10; z++) {
            Utilities::startTimer(500);
            for(uint i=0;i<testCase.sharefilesize;i++){
                client.share(testCase.sharekeywords[i], item[0], user.userID);
            }
            // client.share(testCase.testKeywords[j], testCase.filePairs[testCase.testKeywords[j]][0], user.userID);
            time = Utilities::stopTimer(500);
            cout << "Share Time:" << time << endl;
        }
    }
    cout << "************" << endl;
    return 0;
}
