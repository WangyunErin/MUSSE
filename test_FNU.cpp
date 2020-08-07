#include "src/musse/Owner.h"
#include "src/utils/Utilities.h"
#include "src/musse/Owner.h"
#include "src/musse/OMAPBasedUser.h"
#include "musse/OMAPBasedUser.h"
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
    Server server(testCase.N);
    Owner client(&server, masterKey, NULL);
    OMAPBasedUser user1(&server, user1Key, testCase.K * 2, client.userID, NULL);
    client.registerUser(user1.userID, user1Key, testCase.K * 2, user1.omapkey);

    cout << "Start of FNU" << endl;
    int cnt = 0;
    client.beginSetup();
    vector<pair<prf_type,prf_type>> KeyValues;
    //Inserting the test cases (keyword,file) pairs
    for (auto cur = testCase.filePairs.begin(); cur != testCase.filePairs.end(); cur++) {
        for (unsigned int j = 0; j < cur->second.size(); j++) {
            pair<prf_type,prf_type> keyval = client.share(cur->first, cur->second[j], user1.userID);
            KeyValues.push_back(keyval);
            cnt++;
            if (cnt % 10000 == 0) {
                cout << "Initial Insertion:" << cnt << "/" << to_string(testCase.N) << endl;
            }
        }
    }
    server.update(KeyValues);
    client.endSetup();

    double time = 0;
    cnt = 0;
    for (uint j = 0; j < testCase.Qs.size(); j++) {
        cout << "---------------------" << endl;
        cout << "Result of Operations for DB Size " << testCase.N << endl;
        auto item = testCase.filePairs[testCase.testKeywords[j]];

        // measuring search and update execution times
        cout << "Search for Keyword With " << testCase.Qs[j] << " Results" << endl;
        for (int z = 0; z < 10; z++) {
            user1.searchCommunicationSize = 0;
            Utilities::startTimer(500);
            vector<int> res = user1.search(testCase.testKeywords[j]);
            time = Utilities::stopTimer(500);
            cout << "Search Computation Time (microseconds):" << time << endl;
            cout << "Search Communication Size (Bytes):" << user1.searchCommunicationSize << endl;
            cout << "Number of return item:" << res.size() << endl;
        }
        cout << "Share one document With " << testCase.sharefilesize << " Keywords"<< endl;
        client.updateCommunicationSize = 0;
        // Utilities::startTimer(500);
        client.unshare(item[0], &user1, &testCase);
        // time = Utilities::stopTimer(500);
        // cout << "Unshare Time:" << time << endl;
        // cout << "Unshare Communication Size:" << client.updateCommunicationSize << endl;
        client.updateCommunicationSize = 0;
        Utilities::startTimer(500);
        for(uint i=0;i<testCase.sharefilesize;i++){
            client.share(testCase.sharekeywords[i], item[0], user1.userID);
        }
        // client.share(testCase.testKeywords[j], testCase.filePairs[testCase.testKeywords[j]][0], user1.userID);
        time = Utilities::stopTimer(500);
        cout << "Share Time:" << time << endl;
        cout << "Share Communication Size:" << client.updateCommunicationSize << endl;
    }
    cout << "************" << endl;
    return 0;
}
