#pragma once

#include <string>
#include <unordered_map>
#include <map>

#include "crypto/hash.h"
#include "cryptonote/core/key.h"

namespace cryptonote
{

class ISerializer;

class TimestampBlocksIndex
{
  public:
    TimestampBlocksIndex() = default;

    bool add(uint64_t timestamp, const crypto::Hash &hash);
    bool remove(uint64_t timestamp, const crypto::Hash &hash);
    bool find(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t hashesNumberLimit, std::vector<crypto::Hash> &hashes, uint32_t &hashesNumberWithinTimestamps);
    void clear();

    void serialize(ISerializer &s);

    template <class Archive>
    void serialize(Archive &archive, unsigned int version)
    {
        archive &index;
    }

  private:
    std::multimap<uint64_t, crypto::Hash> index;
};
} // namespace cryptonote