#include <algorithm>
#include "Bid.h"

Bid::Bid() {
    std::fill(id.begin(), id.end(), 0);//fill id with 0,i.e. all elements are set to 0
}

Bid::Bid(string value) {
    std::fill(id.begin(), id.end(), 0);
    std::copy(value.begin(), value.end(), id.begin());//copy string value to id
}

Bid::~Bid() {
}

Bid::Bid(int value) {
    std::fill(id.begin(), id.end(), 0);
    for (int i = 0; i < 4; i++) {
        id[3 - i] = (byte_t) (value >> (i * 8)); //forcing type transfer from 32 bits to 8 bits, only remain the last 8 bits
    }  //copy value to id,value is 32 bits(4 bytes),put each byte tocorresponding id[i]
}

Bid::Bid(std::array<byte_t, ID_SIZE> value) {
    std::copy(value.begin(), value.end(), id.begin());
}

Bid Bid::operator++() {
    id[ID_SIZE - 1]++;
    for (int i = ID_SIZE - 1; i > 0; i--) {
        if (id[i] == 0) {
            id[i - 1]++;
        } else {
            break;
        }
    }
    return *this;//this ptr(pointer), returns a copy of current object(Bid) [return this; means return the address of the object]
}

Bid& Bid::operator=(int other) {
    std::fill(id.begin(), id.end(), 0);
    for (int i = 0; i < 4; i++) {
        id[3 - i] = (byte_t) (other >> (i * 8));
    }
    return *this;//returns the current object(Bid&)
}

bool Bid::operator!=(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return true;
        }
    }
    return false;
}

bool Bid::operator!=(const Bid rhs) const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] != rhs.id[i]) {
            return true;
        }
    }
    return false;
}

bool Bid::operator<(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Bid::operator<(int b) const {
    int result = 0;
    result += id[3];
    result += (id[2] << 8);
    result += id[1] << 16;
    result += id[0] << 24;
    return result < b;
}

bool Bid::operator<=(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Bid::operator>(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Bid::operator>=(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Bid::operator>=(int b) const {
    int result = 0;
    result += id[3];
    result += (id[2] << 8);
    result += id[1] << 16;
    result += id[0] << 24;
    return result >= b;
}

bool Bid::operator==(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return false;
        }
    }
    return true;
}

bool Bid::operator==(const Bid rhs) const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] != rhs.id[i]) {
            return false;
        }
    }
    return true;
}

Bid& Bid::operator=(std::vector<byte_t> other) {
    for (int i = 0; i < ID_SIZE; i++) {
        id[i] = other[i];
    }
    return *this;
}

Bid& Bid::operator=(Bid const &other) {
    for (int i = 0; i < ID_SIZE; i++) {
        id[i] = other.id[i];
    }
    return *this;
}

int Bid::getValue() {
    int result = 0;
    result += id[3];
    result += (id[2] << 8);
    result += id[1] << 16;
    result += id[0] << 24;
    return result;
}

void Bid::setValue(int other) {
    std::fill(id.begin(), id.end(), 0);
    for (int i = 0; i < 4; i++) {
        id[3 - i] = (byte_t) (other >> (i * 8));
    }
}

ostream& operator<<(ostream &o, Bid& bid) {  //output stream, extension of cout, cout can only output int, float etc. normal types
    for (int i = 0; i < ID_SIZE; i++) {
        o << (int) bid.id[i] << "-";
    }
    return o;
}
