// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CryptoNoteBasic.h"
#include "crypto/crypto.h"

namespace cryptonote {

KeyPair generateKeyPair() {
  KeyPair k;
  crypto::generate_keys(k.publicKey, k.secretKey);
  return k;
}

}
