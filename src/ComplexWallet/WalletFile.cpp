#include "WalletFile.h"

#include <fstream>

#include "crypto/chacha8.h"
#include "CryptoNoteConfig.h"
#include "WalletLegacy/KeysStorage.h"
#include "Common/StringTools.h"
#include "Common/StdOutputStream.h"
#include "Common/Base58.h"
#include "CryptoNoteCore/CryptoNoteBasicImpl.h"
#include "CryptoNoteCore/Currency.h"

#include "Serialization/BinaryOutputStreamSerializer.h"
#include "CryptoNoteCore/CryptoNoteSerialization.h"

using namespace Common;
using namespace CryptoNote;

 bool create_wallet_by_keys(std::string &wallet_file, std::string &password,
  std::string &address, std::string &spendKey, std::string &viewKey,
  Logging::LoggerRef &logger) {
  Crypto::SecretKey viewSecretKey;
  Crypto::SecretKey sendSecretKey;
  if (!Common::podFromHex(viewKey, viewSecretKey))
  {
    logger(Logging::ERROR) << "Cannot parse view secret key: " << viewKey;
    return false;
  }
  if (!Common::podFromHex(spendKey, sendSecretKey))
  {
    logger(Logging::ERROR) << "Cannot parse send secret key: " << spendKey;
    return false;
  }

  Crypto::PublicKey sendPubKey;
  Crypto::PublicKey viewPubKey;

  if (!Crypto::secret_key_to_public_key(sendSecretKey, sendPubKey))
  {
    logger(Logging::ERROR) << "Cannot get send public key from secret key: " << spendKey;
    return false;
  }

  if (!Crypto::secret_key_to_public_key(viewSecretKey, viewPubKey))
  {
    logger(Logging::ERROR) << "Cannot get send public key from secret key: " << viewKey;
    return false;
  }

  char keyStore[sizeof(Crypto::PublicKey) * 2];

  memcpy(keyStore, &sendPubKey, sizeof(Crypto::PublicKey));
  memcpy(keyStore + sizeof(Crypto::PublicKey), &viewPubKey, sizeof(Crypto::PublicKey));

  using namespace parameters;
  const uint64_t prefix = CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;
  std::string addressStr = Tools::Base58::encode_addr(prefix, std::string(keyStore, sizeof(Crypto::PublicKey) * 2));
  if (addressStr != address)
  {
    logger(Logging::ERROR) << "Addresses mismatch!" << addressStr;
    return false;
  }

  logger(Logging::INFO) << "Addresses match!";

  CryptoNote::AccountKeys keys;
  keys.address.spendPublicKey = sendPubKey;
  keys.address.viewPublicKey = viewPubKey;
  keys.spendSecretKey = sendSecretKey;
  keys.viewSecretKey = viewSecretKey;

  try
  {
    std::ofstream file;
    try
    {
      file.open(wallet_file, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
      if (file.fail())
      {
        throw std::runtime_error("error opening file: " + wallet_file);
      }
    }
    catch (std::exception &e)
    {
      logger(Logging::INFO) << "exception !" << e.what();
    }
    std::stringstream plainArchive;
    StdOutputStream plainStream(plainArchive);
    CryptoNote::BinaryOutputStreamSerializer serializer(plainStream);

    // Saving Keys;

    CryptoNote::KeysStorage ks;

    ks.creationTimestamp = time(NULL);
    ks.spendPublicKey = keys.address.spendPublicKey;
    ks.spendSecretKey = keys.spendSecretKey;
    ks.viewPublicKey = keys.address.viewPublicKey;
    ks.viewSecretKey = keys.viewSecretKey;

    ks.serialize(serializer, "keys");

    bool hasDetails = false;

    serializer(hasDetails, "has_details");

    std::string cache("");

    serializer.binary(const_cast<std::string &>(cache), "cache");

    std::string plain = plainArchive.str();
    std::string cipher;
    Crypto::chacha8_key key;
    Crypto::cn_context context;
    Crypto::generate_chacha8_key(context, password, key);

    cipher.resize(plain.size());

    Crypto::chacha8_iv iv = Crypto::rand<Crypto::chacha8_iv>();
    Crypto::chacha8(plain.data(), plain.size(), key, iv, &cipher[0]);

    uint32_t version = 1;
    StdOutputStream output(file);
    CryptoNote::BinaryOutputStreamSerializer s(output);
    s.beginObject("wallet");
    s(version, "version");
    s(iv, "iv");
    s(cipher, "data");
    s.endObject();

    file.flush();
    file.close();
    logger(Logging::INFO, Logging::BRIGHT_WHITE) << "Generated new wallet: " << address << std::endl
                               << "view key: " << Common::podToHex(keys.viewSecretKey);
  }
  catch (const std::exception &e)
  {
    logger(Logging::INFO, Logging::BRIGHT_WHITE) << "failed to generate new wallet: " << e.what();
    return false;
  }
  return true;
}