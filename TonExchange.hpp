#pragma once

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>
#include <tvm/dict_map.hpp>
#include <tvm/dict_set.hpp>
#include <tvm/dict_array.hpp>
#include <tvm/replay_attack_protection/timestamp.hpp>

namespace tvm { namespace schema {

using TokenAmount = uint128;
using TokenId = uint128;
using WalletGramsType = uint128;

static constexpr unsigned EXCHANGE_TIMESTAMP_DELAY = 1800;
using exchange_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

//-----------Token balance Object-----------
struct support_token {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
};

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

//--------Order------------
struct order {
  uint256 orlder_no;
  uint256 sell_token_root_address_hex;
  uint128 sell_token_amount;
  uint256 seller_send_token_address_hex;
  uint256 seller_resive_token_address_hex;
  bytes   sell_token_symbol;
  //0-inital ,1-fungible token,2-nonfungible
  uint8   sell_token_type;
  uint256 buy_token_root_address_hex;
  uint128 buy_token_amount;
  uint256 buyer_send_token_address_hex;
  uint256 buyer_resive_token_address_hex;
  bytes   buy_token_symbol;
  uint8   buy_token_type;
  //0:expired,1:put, 2:filled,3:part filled,4:cancle. 
  uint8 order_status;
  //perf_get_timestamp();
  //uint64  expired;  
};


// ===== Exchange Contract ===== //
__interface ITonExchange {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor() = 11;

  // Should be provided pubkey (for external owned wallet) or std addr (for internal owned wallet).
  // The other value must be zero.
  [[internal, external, noaccept, dyn_chain_parse]]
  void regNewToken(uint256 token_wallet_hex,bytes token_symbol) = 12;

  [[getter]]
  uint256 getRootAddress() = 13;

  [[getter]]
  support_token getSupportTokenByRoot(uint256 root_addr_hex) = 14;

  //---------------Customer Funds Manager---------------
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void deposit() = 15;
  [[internal, external, noaccept, dyn_chain_parse]]
  void deposit(uint256 customer_wallet_address_hex,uint256 token_root_hex,bytes tokenName,bytes tokenSymbol,uint8 decimals, int8 token_type,TokenAmount tokenAmount) = 15;
  
  [[getter]]
  customer_token getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) = 16;
  
  [[internal, external, noaccept, dyn_chain_parse]]
  void withdraw(uint256 token_root_hex,int8 token_type,TokenAmount tokenAmount) =17;

  [[internal, external, noaccept, dyn_chain_parse]]
  void withdrawTest(address exchange_wallet_address,address to_wallet_address,TokenAmount tokenAmount) =18;

  [[getter]]
  dict_array<TokenId> getNFFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) = 19;
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void withdrawal () = 16;    

  //----------------execute exchange function--------------------------
  [[internal, external, noaccept, dyn_chain_parse]]
  void putOrder(uint256 sell_token_addr_hex,uint128 sell_amount,uint256 seller_send_address,uint256 seller_resive_address,
  uint256 buy_token_addr_hex,uint128 buy_amount,uint256 buyer_send_address,uint256 buyer_resive_address)=20;

  
};

struct DTonExchange {
  uint256 root_address_hex;
  //support token list : token root addr hex--->token wallet addr hex
  dict_map<uint256,support_token> support_token_list;
  //TIP3 Fungible token  balance list: root token addr hex---> [customer wallet addr hex -->token hold details] 
  dict_map<uint256,dict_map<uint256,customer_token>> token_balance_list;
  //TIP3 NFFungible token  balance list:
  dict_map<uint256,dict_map<uint256,customer_nftoken>> nftoken_balance_list;

  uint256 order_no_count;
  //wait filled ordre list: maker  address->order
  dict_map<uint256,order> order_list;
  // //already done filled order list: taker address->maker order
  dict_map<uint256,uint256> filled_order_list;


 
};

struct ETonExchange {
};


}} // namespace tvm::schema