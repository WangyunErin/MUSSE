#include "Utilities.h"
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <algorithm>


std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock>> Utilities::m_begs;
std::map<int, double> timehist;
unsigned char Utilities::key[16];
unsigned char Utilities::iv[16];

Utilities::Utilities() {
    memset(key, 0x00, 16); //neicun fuzhi function, gei mouyikuai neicunkongjian fuzhi byte by byte (name of array or pointer,value that to be filled,#bytes that to be filled)
    memset(iv, 0x00, 16);
}

Utilities::~Utilities() {
}

void Utilities::startTimer(int id) {
    std::chrono::time_point<std::chrono::high_resolution_clock> m_beg = std::chrono::high_resolution_clock::now();
    m_begs[id] = m_beg;

}

double Utilities::stopTimer(int id) { //duration_cast transform the unit(danwei) of time<mubiaodanwei microseconds>, count()returns the number of time unit
    double t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begs[id]).count();
    timehist.erase(id);
    timehist[id] = t;
    return t;
}

//unsigned char* Utilities::getSHA1(unsigned char* input, size_t length) {
//    unsigned char* digest = new unsigned char[SHA_DIGEST_LENGTH];
//    SHA1(input, length, (unsigned char*) digest);
//    return digest;
//}


static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/')); //isalnum is used to panduan if a char is number or letter(zimu), || is logic operator: or
}

std::string Utilities::base64_encode(const char* bytes_to_encode, unsigned int in_len) { //base64 is the most famous encode mthod to transmit 8bit bytecode, transfer the binary data to 64(ge) printable characters, changing the 3 bytes to 4 bytes
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);//pointer++ biaoshi yiwei, xian yiwei zai quzhi, zhi wu bianhua
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string Utilities::base64_decode(std::string const& encoded_string) {
    size_t in_len = encoded_string.size();
    size_t i = 0;
    size_t j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = static_cast<unsigned char> (base64_chars.find(char_array_4[i]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = static_cast<unsigned char> (base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

std::string Utilities::XOR(std::string value, std::string key) {
    std::string retval(value);//initialize the string retval as value

    short unsigned int klen = key.length();
    short unsigned int vlen = value.length();
    short unsigned int k = 0;
    if (klen < vlen) {
        for (int i = klen; i < vlen; i++) {
            key += " ";
        }
    } else {
        for (int i = vlen; i < klen; i++) {
            value += " ";
        }
    }
    klen = vlen;

    for (short unsigned int v = 0; v < vlen; v++) {
        retval[v] = value[v]^key[k]; // ^ is the bit operator XOR
        k = (++k < klen ? k : 0);
    }

    return retval;
}

std::array<uint8_t, 16> Utilities::convertToArray(std::string addr) {
    std::array<uint8_t, 16> res;
    for (int i = 0; i < 16; i++) {
        res[i] = addr[i];
    }
    return res;
}

int Utilities::getBid(std::string srchIndex) {
    return 0;
}

std::array<uint8_t, 16> Utilities::encode(std::string keyword) {
    unsigned char plaintext[16];
    for (unsigned int i = 0; i < keyword.length(); i++) {
        plaintext[i] = keyword.at(i);
    }
    for (uint i = keyword.length(); i < 16 - 4; i++) {
        plaintext[i] = '\0';
    }

    unsigned char ciphertext[16];
    encrypt(plaintext, strlen((char *) plaintext), key, iv, ciphertext);
    std::array<uint8_t, 16> result;
    for (uint i = 0; i < 16; i++) {
        result[i] = ciphertext[i];
    }
    return result;
}

std::string Utilities::decode(std::array<uint8_t, 16> ciphertext) {
    unsigned char plaintext[16];
    unsigned char cipher[16];
    for (uint i = 0; i < 16; i++) {
        cipher[i] = ciphertext[i];
    }
    decrypt(cipher, 16, key, iv, plaintext);
    std::string result;
    for (uint i = 0; i < 16 && plaintext[i] != '\0'; i++) {
        result += (char) plaintext[i];
    }
    return result;
}

int Utilities::encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        handleErrors();

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

void Utilities::handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

int Utilities::decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv))
        handleErrors();
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

std::vector<std::string> Utilities::split(const std::string& s, char delimiter) { //delimiter , , , 
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

char Utilities::getch() {
    char buf = 0;
    struct termios old = {0};  //zhongduan shebei jiekou
    if (tcgetattr(0, &old) < 0)  //tcgetattr function is used to initialize termios jiegou, get biaoshi zhongduanshebeishezhi de wenjianmiaoshufu fd(=0), fangru termios_p(=&old)suozhixiang de jiegouti zhong
        perror("tcsetattr()");   //jiang shangyige hanshu fashengcuowu de yuanyin shuchu dao biaozhunshebei
    old.c_lflag &= ~ICANON;   //local mode flags, ICANON means allowing canonical mode(shurugongzuo zaihang moshi, shoudao hangdingjiefu hou duqu
    old.c_lflag &= ~ECHO;     //ECHO means to show the shuru de zifu
    old.c_cc[VMIN] = 1;       //VMIN non canonical mode du caozuo zuishao zifushu
    old.c_cc[VTIME] = 0;      //non canonical mode du caozuo chaoshi (unit:0.1s)
    if (tcsetattr(0, TCSANOW, &old) < 0)  //change zhongduan settings, TCSANOW means change immediately
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)   //read function: read n bytes(1 here) from filede=0 zhiding de yidakai file zhong to buf
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

std::vector<std::string> Utilities::splitData(const std::string& str, const std::string& delim) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;  //size_t is unsigned type which is designed to be large enough to store any duixiang in RAM
    do {
        pos = str.find(delim, prev);  //to find the address of delim after prev in string str,return its Subscript(xiabiao)
        if (pos == std::string::npos) pos = str.length();  //npos is a constant to denote the address that does not exist
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

std::string Utilities::executeCommand(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");  //create a pipe to call fork to create a child process, execute a shell yi yunxing mingling to start a process
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) { //read from pipe sizeof buffer-1 characters,  put them into buffer
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

bool Utilities::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);  //,  to , 滻ָ, , ַ, , , , ʼλ, start_pos , ʼ, , Ϊ from.length() , , ַ, , 
    return true;
}

std::string Utilities::random_string(size_t length) {  //, , , , ɳ, , Ϊlength, , , ַ, , 
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0); //ʹ, õ, , ַ, , 0, , ʼ, strΪlength, 0
    std::generate_n( str.begin(), length, randchar ); //, randchar, , length, , ַ, , , str
    return str;
}

unsigned char* Utilities::getSHA256(unsigned char* input, size_t length)
{
    unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, length);
    SHA256_Final(hash, &sha256);
    return hash;
}