#include "util.h"
#include "cryptonote/core/CryptoNoteFormatUtils.h"
#include "cryptonote/core/CryptoNoteTools.h"
#include "cryptonote/core/CryptoNoteSerialization.h"


namespace cryptonote {

bool operator ==(const cryptonote::Block& a, const cryptonote::Block& b) {
  return cryptonote::get_block_hash(a) == cryptonote::get_block_hash(b);
}

bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b) {
  return getObjectHash(a) == getObjectHash(b);
}

}