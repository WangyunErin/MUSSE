#include "src/mkse/MKSEOwner.h"
#include "src/mkse/MKSEServer.h"
#include "src/mkse/MKSEUser.h"
#include "src/utils/Utilities.h"
#include "mkse/MKSEUser.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>

using namespace std;

int main(int, char**) {
    TC<int> testCase;
    uint keywordLength = 16;
    Utilities::readConfigFile("config.txt", testCase);
    Utilities::generateTestCases(testCase, keywordLength, 14);

    unsigned char masterKey[AES_KEY_SIZE], user1Key[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        masterKey[i] = rand() % 256;
        user1Key[i] = rand() % 256;
    }
    cout << "Master Key and Users' Keys Generated" << endl;
    MKSEServer server;
    MKSEOwner client(&server, masterKey);
    MKSEUser user1(&server, user1Key);
    client.registerUser(user1.userID, user1Key);

    cout << "Start of MKSE" << endl;
    int cnt = 0;
    client.beginSetup();
    //Inserting the test cases (keyword,file) pairs
    for (auto cur = testCase.filePairs.begin(); cur != testCase.filePairs.end(); cur++) {
        for (unsigned int j = 0; j < cur->second.size(); j++) {
            client.shareData(cur->first, cur->second[j], user1.userID);
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
        cout << "Search for Keyword With " << testCase.Qs[j] << " Result" << endl;
        for (int z = 0; z < 10; z++) {
            user1.searchCommunicationSize = 0;
            Utilities::startTimer(500);
            vector<int> res = user1.search(testCase.testKeywords[j]);
            time = Utilities::stopTimer(500);
            cout << "Search Computation Time (microseconds):" << time << endl;
            cout << "Search Communication Size (Bytes):" << user1.searchCommunicationSize << endl;
            cout << "Number of return item:" << res.size() << endl;
        }
        // cout << "Share one document With " << testCase.sharefilesize << " Keywords"<< endl;
        // for (int z = 0; z < 10; z++) {      
        //     client.updateCommunicationSize = 0;
        //     Utilities::startTimer(500);
        //     for(uint i=0;i<testCase.sharefilesize;i++){
        //         client.shareData(testCase.sharekeywords[i], item[0], user1.userID);
        //     }
        //     // client.shareData(testCase.testKeywords[j], item[0], user1.userID);
        //     time = Utilities::stopTimer(500);
        //     cout << "Share Time:" << time << endl;
        //     cout << "Share Communication Size:" << client.updateCommunicationSize << endl;
        // }
    }
    cout << "************" << endl;
    return 0;
}
