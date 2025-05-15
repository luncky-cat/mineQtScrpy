#ifndef ADBCRYPTOUTILS_H
#define ADBCRYPTOUTILS_H



// #include "include/openssl/evp.h"
//#include <openssl/evp.h>

#include<openssl/evp.h>
#include <vector>

class AdbCryptoUtils
{
public:
    static AdbCryptoUtils& getInstance();
    ~AdbCryptoUtils();
public:
    std::vector<uint8_t> signAdbTokenPayload(const std::vector<uint8_t> &payload);
    std::vector<uint8_t> getPublicKeyBytes();
    bool generateRsaKeys(int keyLengthBits=2048);   //自定义的公私钥匙
    bool savePrivateKey(const char* filename = "D:/Documents/private.pem");
    bool savePublicKey(const char* filename = "D:/Documents/public.pem");
    bool loadPrivateKey(const char* filename = "D:/Documents/private.pem");
    bool loadPublicKey(const char* filename = "D:/Documents/public.pem");
private:
    EVP_PKEY* private_key_;  // 存储私钥
    EVP_PKEY* public_key_;   // 存储公钥
    static constexpr size_t MY_SHA_DIGEST_LENGTH = 32;

    AdbCryptoUtils();
    AdbCryptoUtils(const AdbCryptoUtils&) = delete;
    AdbCryptoUtils& operator=(const AdbCryptoUtils&) = delete;
};

#endif // ADBCRYPTOUTILS_H
