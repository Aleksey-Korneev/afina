#ifndef AFINA_STORAGE_STRIPED_LRU_H
#define AFINA_STORAGE_STRIPED_LRU_H

#include <memory>
#include <functional>
#include <string>
#include <vector>

#include <afina/Storage.h>
#include "ThreadSafeSimpleLRU.h"


namespace Afina {
namespace Backend {

constexpr std::size_t MIN_STRIPE_SIZE = 1024 * 1024UL;

class StripedLRU : public Afina::Storage {
private:
    StripedLRU(std::size_t stripe_size, std::size_t n_stripes) : _stripes_cnt{n_stripes} {
        _stripes.reserve(n_stripes);
        for (std::size_t i = 0; i < n_stripes; ++i) {
            _stripes.emplace_back(new ThreadSafeSimplLRU(stripe_size));
        }
    }

public:
    static std::unique_ptr<StripedLRU>
    BuildStripedLRU(std::size_t memory_limit = 16 * 1024 * 1024UL,
                    std::size_t stripe_count = 4);

    ~StripedLRU() {}

    // Implements Afina::Storage interface
    bool Put(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool PutIfAbsent(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Set(const std::string &key, const std::string &value) override;

    // Implements Afina::Storage interface
    bool Delete(const std::string &key) override;

    // Implements Afina::Storage interface
    bool Get(const std::string &key, std::string &value) override;

private:
    std::vector<std::unique_ptr<ThreadSafeSimplLRU>> _stripes;
    std::hash<std::string> _hash_stripes;
    std::size_t _stripes_cnt;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_STRIPED_LRU_H
