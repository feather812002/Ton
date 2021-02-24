#include "TonExchange.hpp"
#include "./tokens-fungible/TONTokenWallet.hpp"

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>



using namespace tvm;
using namespace schema;

template<bool Internal>
class TonExchange final : public smart_interface<ITonExchange>, public DTonExchange {
public:
 
  struct error_code : tvm::error_code {
    static constexpr unsigned message_sender_is_not_my_owner  = 100;
    static constexpr unsigned not_enough_balance              = 101;
    static constexpr unsigned add_new_token_no_address        = 102;
    static constexpr unsigned wrong_bounced_args              = 103;
    static constexpr unsigned internal_owner_enabled          = 104;
    static constexpr unsigned internal_owner_disabled         = 105;
    static constexpr unsigned define_pubkey_or_internal_owner = 106;
    static constexpr unsigned wrong_wallet_code_hash          = 107;
  };

  __always_inline
  void constructor() {
  }
  


  __always_inline
  void regNewToken(uint256 token_wallet) {
    //require(token_wallet!=0,error_code::add_new_token_no_address);
    tvm_accept();
    auto sender=int_sender();
    root_address_hex=std::get<addr_std>(sender()).address;
    if(!support_token_list.contains(root_address_hex.get())){
      support_token_list.set_at(root_address_hex.get(), token_wallet);
    }
  }

  

  // getters
  __always_inline uint256 getRootAddress() {
    return root_address_hex;
  }

  __always_inline uint256 getSupportTokenByRoot(uint256 root_addr_hex) {
    uint256 token_wallet=uint256(0);
    if(support_token_list.contains(root_addr_hex.get())){
        token_wallet=support_token_list.get_at(root_addr_hex.get());
    }
    return token_wallet;
  }


  // received bounced message back
  __always_inline static int _on_bounced(cell msg, slice msg_body) {
    tvm_accept();

    
    return 0;
  }

  


  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(ITonExchange, root_replay_protection_t)

  
};

DEFINE_JSON_ABI(ITonExchange, DTonExchange, ETonExchange);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS_TMPL(TonExchange, ITonExchange, DTonExchange, EXCHANGE_TIMESTAMP_DELAY)