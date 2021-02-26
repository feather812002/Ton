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

static constexpr unsigned EXCHANGE_TIMESTAMP_DELAY = 1800;
using root_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

//-----------Token balance Object-----------
struct customer_token {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  uint8 decimals;
  TokenAmount token_balance;
};

struct customer_nftoken {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  dict_set<TokenId>  tokenid_list;
};


// ===== Root Token Contract ===== //
__interface ITonExchange {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor() = 11;

  // Should be provided pubkey (for external owned wallet) or std addr (for internal owned wallet).
  // The other value must be zero.
  [[internal, external, noaccept, dyn_chain_parse]]
  void regNewToken(uint256 token_wallet_hex) = 12;

  [[getter]]
  uint256 getRootAddress() = 13;

  [[getter]]
  uint256 getSupportTokenByRoot(uint256 root_addr_hex) = 14;

  //---------------Customer Funds Manager---------------
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void deposit() = 15;
  [[internal, external, noaccept, dyn_chain_parse]]
  void deposit(uint256 customer_wallet_address_hex,uint256 token_root_hex,bytes tokenName,bytes tokenSymbol,uint8 decimals, int8 token_type,TokenAmount tokenAmount) = 15;
  
  [[getter]]
  customer_token getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) = 16;


 
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void withdrawal () = 16;    

  
};

struct DTonExchange {
  uint256 root_address_hex;
  //support token list : token root addr hex--->token wallet addr hex
  dict_map<uint256,uint256> support_token_list;
  //TIP3 Fungible token  balance list: root token addr hex---> [customer wallet addr hex -->token hold details] 
  dict_map<uint256,dict_map<uint256,customer_token>> token_balance_list;
  //TIP3 NFFungible token  balance list:
  dict_map<uint256,dict_map<uint256,customer_nftoken>> nftoken_balance_list;


 
};

struct ETonExchange {
};


}} // namespace tvm::schema