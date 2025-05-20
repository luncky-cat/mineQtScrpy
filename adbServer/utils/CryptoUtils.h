#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include<openssl/evp.h>
#include <vector>

class CryptoUtils
{
public:
    static CryptoUtils& getInstance();
    ~CryptoUtils();
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
    CryptoUtils();
    CryptoUtils(const CryptoUtils&) = delete;
    CryptoUtils& operator=(const CryptoUtils&) = delete;
};

#endif // CRYPTOUTILS_H
