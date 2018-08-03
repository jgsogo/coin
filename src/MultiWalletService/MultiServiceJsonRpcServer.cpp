// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiServiceJsonRpcServer.h"

#include <functional>
#include <chrono>

#include "MultiServiceJsonRpcMessages.h"

#include "Common/StringTools.h"

#include "Serialization/JsonInputValueSerializer.h"
#include "Serialization/JsonOutputStreamSerializer.h"

#include "./http/errors.h"

using namespace CryptoNote;

using namespace Errors;

namespace MultiWalletService
{

std::string get_time_str()
{
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

MultiServiceJsonRpcServer::MultiServiceJsonRpcServer(System::Dispatcher &sys, System::Event &stopEvent, Logging::ILogger &loggerGroup, WalletInterface &wallet)
    : JsonRpcServer(sys, stopEvent, loggerGroup), logger(loggerGroup, "MultiServiceJsonRpcServer"), m_wallet(wallet)
{
  handlers.emplace("login", jsonHandler<Login::Request, Login::Response>(std::bind(&MultiServiceJsonRpcServer::handleLogin, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("createAddress", jsonHandler<CreateAddress::Request, CreateAddress::Response>(std::bind(&MultiServiceJsonRpcServer::handleCreateAddress, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("deleteAddress", jsonHandler<DeleteAddress::Request, DeleteAddress::Response>(std::bind(&MultiServiceJsonRpcServer::handleDeleteAddress, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getSpendKeys", jsonHandler<GetSpendKeys::Request, GetSpendKeys::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetSpendKeys, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getBalance", jsonHandler<GetBalance::Request, GetBalance::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetBalance, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getBlockHashes", jsonHandler<GetBlockHashes::Request, GetBlockHashes::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetBlockHashes, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getTransactionHashes", jsonHandler<GetTransactionHashes::Request, GetTransactionHashes::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetTransactionHashes, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getTransactions", jsonHandler<GetTransactions::Request, GetTransactions::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetTransactions, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getUnconfirmedTransactionHashes", jsonHandler<GetUnconfirmedTransactionHashes::Request, GetUnconfirmedTransactionHashes::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetUnconfirmedTransactionHashes, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getTransaction", jsonHandler<GetTransaction::Request, GetTransaction::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetTransaction, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("sendTransaction", jsonHandler<SendTransaction::Request, SendTransaction::Response>(std::bind(&MultiServiceJsonRpcServer::handleSendTransaction, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("createDelayedTransaction", jsonHandler<CreateDelayedTransaction::Request, CreateDelayedTransaction::Response>(std::bind(&MultiServiceJsonRpcServer::handleCreateDelayedTransaction, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getDelayedTransactionHashes", jsonHandler<GetDelayedTransactionHashes::Request, GetDelayedTransactionHashes::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetDelayedTransactionHashes, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("deleteDelayedTransaction", jsonHandler<DeleteDelayedTransaction::Request, DeleteDelayedTransaction::Response>(std::bind(&MultiServiceJsonRpcServer::handleDeleteDelayedTransaction, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("sendDelayedTransaction", jsonHandler<SendDelayedTransaction::Request, SendDelayedTransaction::Response>(std::bind(&MultiServiceJsonRpcServer::handleSendDelayedTransaction, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getViewKey", jsonHandler<GetViewKey::Request, GetViewKey::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetViewKey, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getStatus", jsonHandler<GetStatus::Request, GetStatus::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetStatus, this, std::placeholders::_1, std::placeholders::_2)));
  // handlers.emplace("getAddresses", jsonHandler<GetAddresses::Request, GetAddresses::Response>(std::bind(&MultiServiceJsonRpcServer::handleGetAddresses, this, std::placeholders::_1, std::placeholders::_2)));
}

void MultiServiceJsonRpcServer::processJsonRpcRequest(const Common::JsonValue &req, Common::JsonValue &resp)
{
  try
  {
    prepareJsonResponse(req, resp);

    if (!req.contains("method"))
    {
      logger(Logging::WARNING) << "Field \"method\" is not found in json request: " << req;
      makeGenericErrorReponse(resp, "Invalid Request", -3600);
      return;
    }

    if (!req("method").isString())
    {
      logger(Logging::WARNING) << "Field \"method\" is not a string type: " << req;
      makeGenericErrorReponse(resp, "Invalid Request", -3600);
      return;
    }

    std::string method = req("method").getString();

    auto it = handlers.find(method);
    if (it == handlers.end())
    {
      logger(Logging::WARNING) << "Requested method not found: " << method;
      makeMethodNotFoundResponse(resp);
      return;
    }

    logger(Logging::DEBUGGING) << method << " request came";

    Common::JsonValue params(Common::JsonValue::OBJECT);
    if (req.contains("params"))
    {
      params = req("params");
    }

    it->second(params, resp);
  }
  catch (std::exception &e)
  {
    logger(Logging::WARNING) << "Error occurred while processing JsonRpc request: " << e.what();
    makeGenericErrorReponse(resp, e.what());
  }
}

std::error_code MultiServiceJsonRpcServer::handleLogin(const Login::Request &request, Login::Response &response)
{
  if (request.address.empty())
  {
    return make_error_code(MultiWalletErrorCode::INVALID_ADDRESS);
  }
  if (request.sendSecretKey.empty())
  {
    return make_error_code(MultiWalletErrorCode::INVALID_SEND_SECRET_KEY);
  }
  if (request.viewSecretKey.empty())
  {
    return make_error_code(MultiWalletErrorCode::INVALID_VIEW_SECRET_KEY);
  }

  AccountKeys keys;
  logger(Logging::INFO) << "address is " << request.address << ", length" << request.address.size() << endl;
  logger(Logging::INFO) << "sendSecretKey is " << request.sendSecretKey << ", length" << request.sendSecretKey.size() << endl;
  logger(Logging::INFO) << "viewSecretKey is " << request.viewSecretKey << ", length" << request.viewSecretKey.size() << endl;

  // if (!Common::fromHex(request.address, &keys.address, sizeof(keys.address)))
  // {
  //   return make_error_code(MultiWalletErrorCode::INVALID_ADDRESS);
  // }
  logger(Logging::INFO) << "before 2: ";

  if (!Common::fromHex(request.viewSecretKey, &keys.viewSecretKey, sizeof(keys.viewSecretKey)))
  {
    return make_error_code(MultiWalletErrorCode::INVALID_SEND_SECRET_KEY);
  }
  logger(Logging::INFO) << "before 3: ";

  if (!Common::fromHex(request.sendSecretKey, &keys.spendSecretKey, sizeof(keys.spendSecretKey)))
  {
    return make_error_code(MultiWalletErrorCode::INVALID_VIEW_SECRET_KEY);
  }

  secret_key_to_public_key(keys.spendSecretKey, keys.address.spendPublicKey);
  secret_key_to_public_key(keys.viewSecretKey, keys.address.viewPublicKey);

  std::string address = m_wallet.getAddressesByKeys(keys.address);
  logger(Logging::INFO) << "generated address: " << address;

  if (!m_wallet.checkAddress(request.address, keys.address))
  {
    return make_error_code(MultiWalletErrorCode::INVALID_ADDRESS);
  }

  logger(Logging::INFO) << "before 4: ";

  if (!m_wallet.isWalletExisted(address))
  {
    m_wallet.createWallet(keys);
  }

  response.token = m_wallet.sha256(get_time_str());
  logger(Logging::INFO) << "token is: " << response.token;

  logger(Logging::INFO) << "before 5: ";

  return std::error_code();
}

// std::error_code MultiServiceJsonRpcServer::handleCreateAddress(const CreateAddress::Request& request, CreateAddress::Response& response) {
//   if (request.spendSecretKey.empty() && request.spendPublicKey.empty()) {
//     return service.createAddress(response.address);
//   } else if (!request.spendSecretKey.empty()) {
//     return service.createAddress(request.spendSecretKey, response.address);
//   } else {
//     return service.createTrackingAddress(request.spendPublicKey, response.address);
//   }
// }

// std::error_code MultiServiceJsonRpcServer::handleDeleteAddress(const DeleteAddress::Request& request, DeleteAddress::Response& response) {
//   return service.deleteAddress(request.address);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetSpendKeys(const GetSpendKeys::Request& request, GetSpendKeys::Response& response) {
//   return service.getSpendkeys(request.address, response.spendPublicKey, response.spendSecretKey);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetBalance(const GetBalance::Request& request, GetBalance::Response& response) {
//   if (!request.address.empty()) {
//     return service.getBalance(request.address, response.availableBalance, response.lockedAmount);
//   } else {
//     return service.getBalance(response.availableBalance, response.lockedAmount);
//   }
// }

// std::error_code MultiServiceJsonRpcServer::handleGetBlockHashes(const GetBlockHashes::Request& request, GetBlockHashes::Response& response) {
//   return service.getBlockHashes(request.firstBlockIndex, request.blockCount, response.blockHashes);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetTransactionHashes(const GetTransactionHashes::Request& request, GetTransactionHashes::Response& response) {
//   if (!request.blockHash.empty()) {
//     return service.getTransactionHashes(request.addresses, request.blockHash, request.blockCount, request.paymentId, response.items);
//   } else {
//     return service.getTransactionHashes(request.addresses, request.firstBlockIndex, request.blockCount, request.paymentId, response.items);
//   }
// }

// std::error_code MultiServiceJsonRpcServer::handleGetTransactions(const GetTransactions::Request& request, GetTransactions::Response& response) {
//   if (!request.blockHash.empty()) {
//     return service.getTransactions(request.addresses, request.blockHash, request.blockCount, request.paymentId, response.items);
//   } else {
//     return service.getTransactions(request.addresses, request.firstBlockIndex, request.blockCount, request.paymentId, response.items);
//   }
// }

// std::error_code MultiServiceJsonRpcServer::handleGetUnconfirmedTransactionHashes(const GetUnconfirmedTransactionHashes::Request& request, GetUnconfirmedTransactionHashes::Response& response) {
//   return service.getUnconfirmedTransactionHashes(request.addresses, response.transactionHashes);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetTransaction(const GetTransaction::Request& request, GetTransaction::Response& response) {
//   return service.getTransaction(request.transactionHash, response.transaction);
// }

// std::error_code MultiServiceJsonRpcServer::handleSendTransaction(const SendTransaction::Request& request, SendTransaction::Response& response) {
//   return service.sendTransaction(request, response.transactionHash);
// }

// std::error_code MultiServiceJsonRpcServer::handleCreateDelayedTransaction(const CreateDelayedTransaction::Request& request, CreateDelayedTransaction::Response& response) {
//   return service.createDelayedTransaction(request, response.transactionHash);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetDelayedTransactionHashes(const GetDelayedTransactionHashes::Request& request, GetDelayedTransactionHashes::Response& response) {
//   return service.getDelayedTransactionHashes(response.transactionHashes);
// }

// std::error_code MultiServiceJsonRpcServer::handleDeleteDelayedTransaction(const DeleteDelayedTransaction::Request& request, DeleteDelayedTransaction::Response& response) {
//   return service.deleteDelayedTransaction(request.transactionHash);
// }

// std::error_code MultiServiceJsonRpcServer::handleSendDelayedTransaction(const SendDelayedTransaction::Request& request, SendDelayedTransaction::Response& response) {
//   return service.sendDelayedTransaction(request.transactionHash);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetViewKey(const GetViewKey::Request& request, GetViewKey::Response& response) {
//   return service.getViewKey(response.viewSecretKey);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetStatus(const GetStatus::Request& request, GetStatus::Response& response) {
//   return service.getStatus(response.blockCount, response.knownBlockCount, response.lastBlockHash, response.peerCount);
// }

// std::error_code MultiServiceJsonRpcServer::handleGetAddresses(const GetAddresses::Request& request, GetAddresses::Response& response) {
//   return service.getAddresses(response.addresses);
// }

} // namespace MultiWalletService