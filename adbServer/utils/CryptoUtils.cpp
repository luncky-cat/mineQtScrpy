#include "CryptoUtils.h"

#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/pem.h>

#include <iostream>
#include <memory>

CryptoUtils::CryptoUtils() {
    OpenSSL_add_all_algorithms();  // 只在需要时使用
    private_key_ = EVP_PKEY_new();  // 初始化 EVP_PKEY
    public_key_ = EVP_PKEY_new();   // 初始化 EVP_PKEY

    if (!private_key_ || !public_key_) {
        std::cerr << "Failed to initialize EVP_PKEY objects!" << std::endl;
        return;  // 结束构造函数，避免继续执行后续操作
    }

    // 尝试加载公钥和私钥
    if (loadPublicKey() && loadPrivateKey()) {
        // 成功加载
        std::cerr << "Keys loaded successfully." << std::endl;
    } else {
        std::cerr << "Key loading failed. Generating new RSA keys..." << std::endl;
        if (!generateRsaKeys()) {
            std::cerr << "Failed to generate RSA keys!" << std::endl;
            return;  // 如果生成失败，直接返回
        }
        std::cerr << "RSA keys generated successfully." << std::endl;
    }
}

CryptoUtils &CryptoUtils::getInstance()
{
    static CryptoUtils instance; // C++11 线程安全懒汉式单例
    return instance;
}

CryptoUtils::~CryptoUtils() {
    // 对私钥进行处理
    if (private_key_) {
        if (!savePrivateKey()) {
            std::cerr << "Failed to save private key!" << std::endl;
        }
        EVP_PKEY_free(private_key_);
        private_key_ = nullptr;  // 确保指针被清空，避免重复释放
    }

    // 对公钥进行处理
    if (public_key_) {
        if (!savePublicKey()) {
            std::cerr << "Failed to save public key!" << std::endl;
        }
        EVP_PKEY_free(public_key_);
        public_key_ = nullptr;  // 确保指针被清空，避免重复释放
    }
}


bool CryptoUtils::generateRsaKeys(int keyLengthBits) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        std::cerr << "EVP_PKEY_CTX_new_id failed!" << std::endl;
        return false;
    }

    // 初始化密钥生成
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "EVP_PKEY_keygen_init failed!" << std::endl;
        return false;
    }

    // 设置密钥长度
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keyLengthBits) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "EVP_PKEY_CTX_set_rsa_keygen_bits failed!" << std::endl;
        return false;
    }

    // 生成密钥
    if (EVP_PKEY_keygen(ctx, &private_key_) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "EVP_PKEY_keygen failed!" << std::endl;
        return false;
    }

    // 复制私钥生成公钥
    public_key_ = EVP_PKEY_dup(private_key_);
    if (public_key_ == nullptr) {
        EVP_PKEY_CTX_free(ctx);
        std::cerr << "EVP_PKEY_dup failed!" << std::endl;
        return false;
    }

    // 清理上下文
    EVP_PKEY_CTX_free(ctx);

    return true;
}

bool CryptoUtils::savePrivateKey(const char* filename)
{
    BIO* bio = BIO_new_file(filename, "wb");
    if (bio == nullptr) {
        std::cerr << "Failed to open file for saving private key: " << filename << std::endl;
        return false;
    }

    if (PEM_write_bio_PrivateKey(bio, private_key_, nullptr, nullptr, 0, nullptr, nullptr) == 0) {
        std::cerr << "Failed to write private key to file: " << filename << std::endl;
        BIO_free(bio);
        return false;
    }

    BIO_free(bio);
    return true;
}

bool CryptoUtils::savePublicKey(const char* filename)
{
    BIO* bio = BIO_new_file(filename, "wb");
    if (bio == nullptr) {
        std::cerr << "Failed to open file for saving public key: " << filename << std::endl;
        return false;
    }

    if (PEM_write_bio_PUBKEY(bio, public_key_) == 0) {
        std::cerr << "Failed to write public key to file: " << filename << std::endl;
        BIO_free(bio);
        return false;
    }

    BIO_free(bio);
    return true;
}

bool CryptoUtils::loadPrivateKey(const char* filename)
{
    BIO* bio = BIO_new_file(filename, "rb");
    if (bio == nullptr) {
        std::cerr << "Failed to open file for loading private key: " << filename << std::endl;
        return false;
    }

    private_key_ = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (private_key_ == nullptr) {
        std::cerr << "Failed to load private key from file: " << filename << std::endl;
        return false;
    }

    return true;
}

bool CryptoUtils::loadPublicKey(const char* filename)
{
    BIO* bio = BIO_new_file(filename, "rb");
    if (bio == nullptr) {
        std::cerr << "Failed to open file for loading public key: " << filename << std::endl;
        return false;
    }

    public_key_ = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (public_key_ == nullptr) {
        std::cerr << "Failed to load public key from file: " << filename << std::endl;
        return false;
    }

    return true;
}


std::vector<uint8_t> CryptoUtils::signAdbTokenPayload(const std::vector<uint8_t>& payload) {
    unsigned char hash[MY_SHA_DIGEST_LENGTH];
    // 计算 SHA1 摘要
    if (SHA1(payload.data(), payload.size(), hash) == nullptr) {
        std::cerr << "SHA1 hash failed!" << std::endl;
        return {};
    }

    // 创建 EVP PKEY 上下文
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx(EVP_PKEY_CTX_new(private_key_, nullptr), EVP_PKEY_CTX_free);
    if (ctx == nullptr) {
        std::cerr << "Failed to create EVP context!" << std::endl;
        return {};
    }

    // 初始化签名操作
    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        std::cerr << "EVP_PKEY_sign_init failed" << std::endl;
        return {};
    }

    // 设置 RSA 填充方式
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) <= 0) {
        std::cerr << "EVP_PKEY_CTX_set_rsa_padding failed" << std::endl;
        return {};
    }

    // 获取签名的大小
    size_t sig_len = EVP_PKEY_size(private_key_);
    std::vector<unsigned char> sig(sig_len);

    // 执行签名
    if (EVP_PKEY_sign(ctx.get(), sig.data(), &sig_len, hash, MY_SHA_DIGEST_LENGTH) <= 0) {
        std::cerr << "EVP_PKEY_sign failed" << std::endl;
        return {};
    }

    sig.resize(sig_len);
    return sig;
}

std::vector<uint8_t> CryptoUtils::getPublicKeyBytes() {
    // 先计算出公钥序列化后的长度
    int pubkey_len = i2d_PUBKEY(public_key_, nullptr);
    if (pubkey_len <= 0) {
        std::cerr << "Failed to extract public key length!" << std::endl;
        return {};  // 返回空向量表示失败
    }

    // 直接在内存中分配空间
    std::vector<unsigned char> public_key_bytes(pubkey_len);

    // 将公钥序列化到分配的内存空间中
    unsigned char* pubkey_ptr = public_key_bytes.data();
    if (i2d_PUBKEY(public_key_, &pubkey_ptr) != pubkey_len) {
        std::cerr << "Failed to serialize public key!" << std::endl;
        return {};  // 序列化失败时返回空向量
    }

    return public_key_bytes;
}
