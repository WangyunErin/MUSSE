#ifndef UTILITIES_H
#define UTILITIES_H
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <chrono>
#include <stdlib.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <math.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <termios.h>
#include <array>
#include <stdexcept>
#include <stdio.h>
#include <set>

#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE 32
typedef std::array<uint8_t, AES_KEY_SIZE> prf_type;
#endif

template <typename T>
class TC {
public:
    uint N;
    uint K;
    uint numOfFiles;
    uint sharefilesize;
    std::vector<uint> Qs;
    //std::vector<uint> delNumber;
    std::vector<std::string> keywords;
    std::vector<std::string> sharekeywords;
    std::vector<std::string> testKeywords;
    std::map<std::string, std::vector<T> > filePairs;
};

class Utilities {
private:
    static int parseLine(char* line);
public:
    Utilities();
    //    static unsigned char* getSHA1(unsigned char* input, size_t length);
    static unsigned char* getSHA256(unsigned char* input, size_t length);
    static std::string base64_encode(const char* bytes_to_encode, unsigned int in_len);
    static std::string base64_decode(std::string const& enc);
    static std::string XOR(std::string value, std::string key);
    static void startTimer(int id);
    static double stopTimer(int id);
    static std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock> > m_begs;
    static std::array<uint8_t, 16> convertToArray(std::string value);
    static int getBid(std::string srchIndex);
    static std::array<uint8_t, 16> encode(std::string keyword);
    static std::string decode(std::array<uint8_t, 16> data);
    static unsigned char key[16], iv[16];
    static int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext);
    static int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
    static void handleErrors(void);
    static std::vector<std::string> split(const std::string& s, char delimiter);
    static char getch();
    static std::vector<std::string> splitData(const std::string& str, const std::string& delim);
    static std::string executeCommand(const char* cmd);
    static bool replace(std::string& str, const std::string& from, const std::string& to);
    static std::string random_string(size_t length);

    template <typename T>
    static void readConfigFile(std::string address, TC<T>& testCase) {
        std::ifstream infile;
        std::string tmp;

        infile.open(address);  //open file address
        getline(infile, tmp);  //read one line and put it into tmp, line 1 is setting N
        testCase.N = stoi(tmp); //transfer the string to int and output
        getline(infile, tmp);  //line 2 is setting K
        testCase.K = stoi(tmp);
        getline(infile, tmp);   //line 3 is setting #files
        testCase.numOfFiles = stoi(tmp);
        getline(infile, tmp);   //line 4 is setting qNumber
        int qNum = stoi(tmp);
        for (int i = 0; i < qNum; i++) {
            getline(infile, tmp); //line 5,7,...
            testCase.Qs.push_back(stoi(tmp)); //pushback add one element to the end, Qs is to store the number of result of each query
            //getline(infile, tmp);  //line 6,8,...
            //testCase.delNumber.push_back(stoi(tmp));
        }
        getline(infile,tmp);
        testCase.sharefilesize=stoi(tmp);
    };

    template <typename T>
    static void generateTestCases(TC<T>& testCase, uint keywordLength, unsigned int seed) {
        char alphanum[] =
                "0123456789"
                "!@#$%^&*"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        srand(seed);
        uint totalKeywordSize = 0;
        uint totalPairNumber = 0;

        for (uint j = 0; j < testCase.K - totalKeywordSize; j++) {
            std::string str;
            for (uint k = 0; k < keywordLength; ++k) {
                str += alphanum[rand() % (sizeof (alphanum) - 1)];
            }
            testCase.keywords.push_back(str);//testCase.keywords to store all keywords(K ge)
        }  //generating K keywords
        totalKeywordSize += testCase.keywords.size();

        for (uint j = 0; j < testCase.Qs.size(); j++) {
            testCase.testKeywords.push_back(testCase.keywords[j]); //testCase.testKeywords stores the first query size ge keywords from testCase.keywords
        }

        for (uint j = 0; j < testCase.sharefilesize; j++) {
            testCase.sharekeywords.push_back(testCase.keywords[j]); //testCase.shareeywords stores sharefilesize keywords from testCase.keywords
        }

        std::set<int> uniqueFiles;
        for (uint j = 0; j < testCase.Qs.size(); j++) { //to build result file id set for each query 
            std::vector<T> files;
            for (uint k = 0; k < testCase.Qs[j]; k++) {
                files.push_back(k);
		uniqueFiles.insert(k);//put each id that has just been created to the set
                totalPairNumber++;  //keyword-id pairs
            }
            testCase.filePairs[testCase.testKeywords[j]] = files;
            files.clear();
        }

        uint totalCounter = totalPairNumber;
        std::vector<T> files;
        while(uniqueFiles.size()<testCase.numOfFiles){
            int fileName = ((rand()%10000000)) + 10000000;
            uniqueFiles.insert(fileName);
        }
        files.insert(files.end(), uniqueFiles.begin(), uniqueFiles.end());//insert all elements in uniqueFiles to files
        for (uint j = testCase.testKeywords.size(); j < testCase.keywords.size(); j++) {
            testCase.filePairs[testCase.keywords[j]] = std::vector<T>();
            testCase.filePairs[testCase.keywords[j]].push_back(files[0]);
            totalCounter++;
        }
        
        for (uint j = testCase.testKeywords.size(); j < testCase.keywords.size(); j++) {
            for (uint i = 1; i < files.size(); i++) {
                testCase.filePairs[testCase.keywords[j]].push_back(files[i]);
                totalCounter++;
                if (totalCounter == testCase.N) {
                    return;
                }
            }
        }

    };
    virtual ~Utilities();
};

#endif /* UTILITIES_H */

