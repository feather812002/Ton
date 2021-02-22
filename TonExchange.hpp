#pragma once

#include <tvm/schema/message.hpp>
#include "./tokens-fungible/TONTokenWallet.hpp"
#include <tvm/sequence.hpp>

#include <tvm/replay_attack_protection/timestamp.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/dict_map.hpp>

namespace tvm { namespace schema {

static constexpr unsigned EXCHANGE_TIMESTAMP_DELAY = 1800;
using wallet_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

struct TokenBalance {
  bytes token_name;
  bytes token_symbol;
  uint128 token_balance;
};
// TonExchang interface
__interface ITonExchange {
  // Handle external messages only
  __attribute__((external,dyn_chain_parse))
  void constructor() = 1;

  //-------------Customer manage function----------------
  // add a new TIP-3 token to exchange
  __attribute__((internal, external, noaccept, dyn_chain_parse))
  void addNewToken(address root_address,uint128 grams,uint256 publicKey,uint256 internal_owner) = 2;
  
  // //desposit TIP-3 token to exchange wallet
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 despositToExchange() = 3;

  // //withdraw TIP-3 token from exchange wallet
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 withdrawFromExchange() = 4;

  // //-----------Exchange function------------------------------- 
  // //put a new order into the exchange 
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 putOrder() = 5;

  // //cancle the order from the exchange
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 cancleOrder() = 6;

  // //list all vaild order from the exchange
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 listOrder() = 7;

  // //fill a order in the exchange
  // __attribute__((internal, external, noaccept, dyn_chain_parse))
  // uint8 fillOrder() = 8;

};

// TonExchange persistent data
struct DTonExchange {
  //TIP-3 token list: root address-> exchange wallet address
  dict_map<uint256,uint256>  tokenWalletListMap;
  //Customer token balance, customer address -> token address: TokenBalance
  dict_map<uint256,dict_map<uint256,TokenBalance>> customer_balance_list;
  
};


struct ETonExchange {};

}} // namespace tvm::schema