#include "src/utils/Utilities.h"
#include "src/mkse/MKSEOwner.h"
#include "src/mkse/MKSEUser.h"
#include "src/mkse/MKSEOwnerRunner.h"
#include "src/mkse/MKSEUserRunner.h"
using namespace std;

int main(int, char**) {
    TC<int> testCase;
    uint keywordLength = 16;
    string serverAdr = "18.217.144.218:4241";
    Utilities::readConfigFile("config.txt", testCase);
    Utilities::generateTestCases(testCase, keywordLength, 14);

    unsigned char masterKey[AES_KEY_SIZE], user1Key[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        masterKey[i] = rand() % 256;
        user1Key[i] = rand() % 256;
    }
    cout << "Master Key and Users' Keys Generated" << endl;

    MKSEOwnerRunner client(serverAdr, masterKey);
    MKSEUserRunner userRunner(serverAdr);
    MKSEUser user(NULL, user1Key, &userRunner);

    client.registerUser(user.userID, user1Key);

    cout << "Start of MKSE" << endl;
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
   // int queueSize = 100;
    for (uint j = 0; j < testCase.Qs.size(); j++) {
        cout << "---------------------" << endl;
        cout << "Result of Operations for DB Size " << testCase.N << endl;
        auto item = testCase.filePairs[testCase.testKeywords[j]];

        //measuring search and update execution times
        cout << "Search for Keyword With " << testCase.Qs[j] << " Result:" << endl;

        for (int z = 0; z < 10; z++) {
            Utilities::startTimer(500);
            vector<int> res = userRunner.search(testCase.testKeywords[j], &user);
            time = Utilities::stopTimer(500);
            cout << "Search Computation Time (microseconds):" << time << endl;
            cout << "Number of return item:" << res.size() << endl;
        }

        cout << "Share one document With " << testCase.sharefilesize << " Keywords"<< endl;
        for (int z = 0; z < 10; z++) {
            Utilities::startTimer(500);
            client.sharedata(testCase.sharekeywords, item[0], user.userID);
            time = Utilities::stopTimer(500);
            cout << "Share Time:" << time << endl;
        }
    }
    cout << "************" << endl;
    return 0;
}
