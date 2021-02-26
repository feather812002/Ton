#pragma once

#include <tvm/schema/message.hpp>
#include <tvm/sequence.hpp>

#include <tvm/replay_attack_protection/timestamp.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/dict_map.hpp>

namespace tvm { namespace schema {

using WalletGramsType = uint128;
using TokensType = uint128;
using TokensAmount = uint128;

static constexpr unsigned TOKEN_WALLET_TIMESTAMP_DELAY = 1800;
using wallet_replay_protection_t = replay_attack_protection::timestamp<TOKEN_WALLET_TIMESTAMP_DELAY>;


// ===== TON Token wallet ===== //
__interface ITONTokenWallet {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor(bytes name, bytes symbol, uint8 decimals,
                   uint256 root_public_key, uint256 wallet_public_key,
                   address root_address, cell code) = 11;

  [[internal, external, noaccept, dyn_chain_parse]]
  void transfer(address dest, TokensType tokens, WalletGramsType grams) = 12;

  [[internal, noaccept, answer_id]]
  TokensType getBalance_InternalOwner() = 13;

  // Receive tokens from root
  [[internal, noaccept]]
  void accept(TokensType tokens) = 14;

  // Receive tokens from other wallet
  [[internal, noaccept]]
  void internalTransfer(TokensType tokens, uint256 pubkey, uint256 my_owner_addr) = 15;

  // Send rest funds to `dest` and destroy the wallet
  [[internal, external, noaccept, dyn_chain_parse]]
  void destroy(address dest) = 16;

  // getters
  [[getter]]
  bytes getName() = 17;

  [[getter]]
  bytes getSymbol() = 18;

  [[getter]]
  uint8 getDecimals() = 19;

  [[getter]]
  TokensType getBalance() = 20;

  [[getter]]
  uint256 getWalletKey() = 21;

  [[getter]]
  address getRootAddress() = 22;

  [[getter]]
  address getOwnerAddress() = 23;

  [[getter]]
  TokensType allowance(uint256 spenderAddressHex) = 24;

  // allowance interface
  [[internal, external, noaccept, dyn_chain_parse]]
  void approve(address spender, TokensType tokens) = 25;

  [[internal, external, noaccept, dyn_chain_parse]]
  void transferFrom(address dest, address to, TokensType tokens,
                    WalletGramsType grams) = 26;

  [[internal]]
  void internalTransferFrom(address to, TokensType tokens) = 27;

  [[internal, external, noaccept]]
  void disapprove() = 28;

  [[getter]]
  TokensType getSenderBalance(uint256 senderAddressHex) = 29;

  // [[getter]]
  // uint8 getTokenType() = 30;
  [[getter]]
  uint256 getWalletCodeHash() = 30;

 

  //----------exchange function -------------------------
  [[internal, external, noaccept, dyn_chain_parse]]
  void depositToExchange(address exchange_address) = 31;

  [[internal, external, noaccept, dyn_chain_parse]]
  void sendDepositToExchangeRequst(address exchange_wallet_address,address exchange_address,WalletGramsType grams_exchange) = 32;

  [[internal, external, noaccept, dyn_chain_parse]]
  void approveTest(address spender, TokensType tokens) = 33;

  [[internal, external, noaccept, dyn_chain_parse]]
  void regTokenToExchangeFromRoot(address exchange_address,WalletGramsType grams) = 34;

  [[internal, external, noaccept, dyn_chain_parse]]
  void sendTransaction(address dest,WalletGramsType grams,cell msgBody) = 35;

  
};

struct DTONTokenWallet {
  bytes name_;
  bytes symbol_;
  uint8 decimals_;
  TokensType balance_;
  uint256 root_public_key_;
  uint256 wallet_public_key_;
  address root_address_;
  std::optional<address> owner_address_;
  cell code_;
  dict_map<uint256,TokensType> allowance_;
  int8 workchain_id_;
  dict_map<uint256,TokensType> balance_list;
  
  //test
  //TokensType approve_total_;
};

struct ETONTokenWallet {
};

// Prepare Token Wallet StateInit structure and expected contract address (hash from StateInit)
inline
std::pair<StateInit, uint256> prepare_wallet_state_init_and_addr(DTONTokenWallet wallet_data) {
  cell wallet_data_cl =
    prepare_persistent_data<ITONTokenWallet, wallet_replay_protection_t, DTONTokenWallet>(
      wallet_replay_protection_t::init(), wallet_data);
  StateInit wallet_init {
    /*split_depth*/{}, /*special*/{},
    wallet_data.code_, wallet_data_cl, /*library*/{}
  };
  cell wallet_init_cl = build(wallet_init).make_cell();
  return { wallet_init, uint256(tvm_hash(wallet_init_cl)) };
}

}} // namespace tvm::schema

