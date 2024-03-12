#pragma once

#include <fstream>
#include <sstream>
#include "cryptopp/cryptlib.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"

#define endl '\n'

#define ROOT "/"
#define INDEX "/index.html"
#define SESSION_LOGGED_IN "logged_in"
#define SESSION_USERNAME "username"
#define SESSION_PASSWORD "password"


inline std::string file_to_string(const std::string &filename)
{
    std::ifstream fStream(filename);
    std::stringstream strStream;
    strStream << fStream.rdbuf();
    return strStream.str();
}

inline std::string sha256(const std::string &input)
{
    using namespace CryptoPP;

    SHA256 hash;
    std::string digest;
    StringSource s(input, true, new HashFilter(hash, new HexEncoder(new StringSink(digest))));
    return digest;
}
