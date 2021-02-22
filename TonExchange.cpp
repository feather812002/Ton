#include "TonExchange.hpp"
#include "./tokens-fungible/TONTokenWallet.hpp"
#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/replay_attack_protection/timestamp.hpp>

using namespace tvm::schema;
using namespace tvm;

class TonExchange final : public smart_interface<ITonExchange>,
                         public DTonExchange {
public:
  struct error_code : tvm::error_code {
    static constexpr unsigned message_sender_is_not_my_owner    = 100;
  };
  /// Deploy the contract.
  __always_inline void constructor() final {}

  //add a new token into
  __always_inline void addNewToken(address root_address)  
  { 
    
  }


  // Function is called in case of unparsed or unsupported func_id
  static __always_inline int _fallback(cell msg, slice msg_body) 
  { return 0; };
};
DEFINE_JSON_ABI(ITonExchange, DTonExchange, ETonExchange);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS(TonExchange, ITonExchange, DTonExchange, TOKEN_WALLET_TIMESTAMP_DELAY)
