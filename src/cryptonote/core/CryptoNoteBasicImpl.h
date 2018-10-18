// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "common/StringTools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote/core/CryptoNoteBasic.h"


namespace cryptonote {
  /************************************************************************/
  /*                                                                      */
  /************************************************************************/
  template<class t_array>
  struct array_hasher: std::unary_function<t_array&, size_t>
  {
    size_t operator()(const t_array& val) const
    {
      return boost::hash_range(&val.data[0], &val.data[sizeof(val.data)]);
    }
  };

  /************************************************************************/
  /* cryptonote.helper functions                                          */
  /************************************************************************/
  uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize);
  std::string getAccountAddressAsStr(uint64_t prefix, const AccountPublicAddress& adr);
  bool parseAccountAddressString(uint64_t& prefix, AccountPublicAddress& adr, const std::string& str);
  bool is_coinbase(const Transaction& tx);

  bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b);
  bool operator ==(const cryptonote::Block& a, const cryptonote::Block& b);
}

template <class T>
std::ostream &print256(std::ostream &o, const T &v) {
  return o << "<" << Common::podToHex(v) << ">";
}

bool parse_hash256(const std::string& str_hash, crypto::Hash& hash);

namespace crypto {
  inline std::ostream &operator <<(std::ostream &o, const crypto::PublicKey &v) { return print256(o, v); }
  inline std::ostream &operator <<(std::ostream &o, const crypto::SecretKey &v) { return print256(o, v); }
  inline std::ostream &operator <<(std::ostream &o, const crypto::KeyDerivation &v) { return print256(o, v); }
  inline std::ostream &operator <<(std::ostream &o, const crypto::KeyImage &v) { return print256(o, v); }
  inline std::ostream &operator <<(std::ostream &o, const crypto::Signature &v) { return print256(o, v); }
  inline std::ostream &operator <<(std::ostream &o, const crypto::Hash &v) { return print256(o, v); }
}
