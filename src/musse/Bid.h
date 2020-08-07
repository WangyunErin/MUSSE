#ifndef BID_H
#define BID_H
#include "Types.hpp"
#include <array>
#include <string>
using namespace std;

class Bid {
public:
    std::array< byte_t, ID_SIZE> id;
    Bid();
    Bid(int value);
    Bid(std::array< byte_t, ID_SIZE> value);
    Bid(string value);
    ~Bid();
    Bid operator++();
    Bid& operator=(int other);
    bool operator!=(const int rhs) const;
    bool operator!=(const Bid rhs) const;
    bool operator==(const int rhs) const;
    bool operator==(const Bid rhs) const;
    Bid& operator=(std::vector<byte_t> other);
    Bid& operator=(Bid const &other);
    bool operator<(const Bid& b) const;
    bool operator<(int b) const;
    bool operator>(const Bid& b) const;
    bool operator<=(const Bid& b) const;
    bool operator>=(const Bid& b) const;
    bool operator>=(int b) const;
    int getValue();
    void setValue(int value);
    friend ostream& operator<<(ostream &o, Bid& id);
};

struct BidHasher {

    std::size_t operator()(const Bid &key) const {
        std::hash<byte_t> hasher;
        size_t result = 0;
        for (size_t i = 0; i < ID_SIZE; ++i) {
            result = (result << 1) ^ hasher(key.id[i]);
        }
        return result;
    }
};

#endif /* BID_H */

