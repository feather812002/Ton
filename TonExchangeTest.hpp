#pragma once

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>
#include <tvm/dict_map.hpp>
#include <tvm/dict_set.hpp>
#include <tvm/replay_attack_protection/timestamp.hpp>

namespace tvm { namespace schema {

using TokenAmount = uint128;
using TokenId = uint128;
using WalletGramsType = uint128;


using root_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

//-----------Token balance Object-----------



// ===== Root Token Contract ===== //
__interface ITonExchangeTest {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor() = 11;

  // Should be provided pubkey (for external owned wallet) or std addr (for internal owned wallet).
  // The other value must be zero.
   [[internal, external, noaccept, dyn_chain_parse]]
  void withdrawTest(address exchange_address,address exchange_wallet_address,address to_wallet_address,TokenAmount tokenAmount,WalletGramsType grams) =12;

  
  
};

struct DTonExchangeTest {
  uint256 root_address_hex;
  //support token list : token root addr hex--->token wallet addr hex
  dict_map<uint256,uint256> support_token_list;
  //TIP3 Fungible token  balance list: root token addr hex---> [customer wallet addr hex -->token hold details] 
  dict_map<uint256,dict_map<uint256,customer_token>> token_balance_list;
  //TIP3 NFFungible token  balance list:
  dict_map<uint256,dict_map<uint256,customer_nftoken>> nftoken_balance_list;


 
};

struct ETonExchangeTest {
};


}} // namespace tvm::schema