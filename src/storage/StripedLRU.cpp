#include "StripedLRU.h"
#include <stdexcept>



namespace Afina {
namespace Backend {
std::unique_ptr<StripedLRU>
StripedLRU::BuildStripedLRU(std::size_t memory_limit, std::size_t stripe_count) {
    std::size_t stripe_size = memory_limit / stripe_count;
    if (stripe_size < MIN_STRIPE_SIZE) {
        throw std::runtime_error("Too low stripe size");
    }
    return std::unique_ptr<StripedLRU>(new StripedLRU(stripe_size, stripe_count));
}

// Implements Afina::Storage interface
bool StripedLRU::Put(const std::string &key, const std::string &value) {
    return _stripes[_hash_stripes(key) % _stripes_cnt]->Put(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    return _stripes[_hash_stripes(key) % _stripes_cnt]->PutIfAbsent(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Set(const std::string &key, const std::string &value) {
    return _stripes[_hash_stripes(key) % _stripes_cnt]->Set(key, value);
}

// Implements Afina::Storage interface
bool StripedLRU::Delete(const std::string &key) {
   return _stripes[_hash_stripes(key) % _stripes_cnt]->Delete(key);
}

// Implements Afina::Storage interface
bool StripedLRU::Get(const std::string &key, std::string &value) {
   return _stripes[_hash_stripes(key) % _stripes_cnt]->Get(key, value);
}

} // namespace Backend
} // namespace Afina
