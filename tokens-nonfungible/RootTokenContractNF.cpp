#include "RootTokenContractNF.hpp"
#include "TONTokenWalletNF.hpp"

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>

#include "../TonExchange.hpp"

using namespace tvm;
using namespace schema;

static constexpr unsigned ROOT_TIMESTAMP_DELAY = 1800;

class RootTokenContract final : public smart_interface<IRootTokenContract>, public DRootTokenContract {
public:
  using root_replay_protection_t = replay_attack_protection::timestamp<ROOT_TIMESTAMP_DELAY>;

  struct error_code : tvm::error_code {
    static constexpr unsigned message_sender_is_not_my_owner = 100;
    static constexpr unsigned token_not_minted               = 101;
    static constexpr unsigned wrong_bounced_header           = 102;
    static constexpr unsigned wrong_bounced_args             = 103;
    static constexpr unsigned wrong_mint_token_id            = 104;
    static constexpr unsigned define_pubkey_or_internal_owner = 105;
  };

  __always_inline
  void constructor(bytes name, bytes symbol, uint8 decimals, uint256 root_public_key, cell wallet_code) {
  
    name_ = name;
    symbol_ = symbol;
    decimals_ = decimals;
    root_public_key_ = root_public_key;
    wallet_code_ = wallet_code;
    total_supply_ = TokensType(0);
    total_granted_ = TokensType(0);
  }

  __always_inline
  lazy<MsgAddressInt> deployWallet(int8 workchain_id, uint256 pubkey, TokenId tokenId, WalletGramsType grams) {
    require(root_public_key_ == tvm_pubkey(), error_code::message_sender_is_not_my_owner);
    require(!tokenId || tokens_.contains(tokenId), error_code::token_not_minted);

    tvm_accept();

    auto [wallet_init, dest] = calc_wallet_init(workchain_id, pubkey,uint256(0));
    contract_handle<ITONTokenWallet> dest_handle(dest);
    dest_handle.deploy(wallet_init, Grams(grams.get())).
      call<&ITONTokenWallet::accept>(tokenId);

    if (tokenId)
      ++total_granted_;
    return dest;
  }

  __always_inline
  address deployEmptyWallet(int8 workchain_id, uint256 pubkey,WalletGramsType grams,uint256 owner_addr) {
  
    // This protects from spending root balance to deploy message
    // if constexpr (Internal) {
    //   auto value_gr = int_value();
    //   tvm_rawreserve((tvm_balance() - value_gr()), rawreserve_flag::up_to);
    // }
     require(pubkey != 0,error_code::define_pubkey_or_internal_owner);
     tvm_accept();
    
    auto [wallet_init, dest] = calc_wallet_init(workchain_id, pubkey,owner_addr);
    handle<ITONTokenWallet> dest_handle(dest);

    dest_handle.deploy_noop(wallet_init, Grams(grams.get()));
    set_int_return_flag(SEND_ALL_GAS);
      
    return dest;
  }

  __always_inline
  void grant(lazy<MsgAddressInt> dest, TokenId tokenId, WalletGramsType grams) {
    require(root_public_key_ == tvm_pubkey(), error_code::message_sender_is_not_my_owner);
    require(tokens_.contains(tokenId), error_code::token_not_minted);

    tvm_accept();

    contract_handle<ITONTokenWallet> dest_handle(dest);
    dest_handle(Grams(grams.get())).call<&ITONTokenWallet::accept>(tokenId);

    ++total_granted_;
  }

  __always_inline
  TokenId mint(TokenId tokenId) {
    require(root_public_key_ == tvm_pubkey(), error_code::message_sender_is_not_my_owner);
    require(tokenId == total_supply_ + 1, error_code::wrong_mint_token_id);

    tvm_accept();

    tokens_.insert(tokenId);
    ++total_supply_;
    return tokenId;
  }

  // getters
  __always_inline bytes getName() {
    return name_;
  }

  __always_inline bytes getSymbol() {
    return symbol_;
  }

  __always_inline uint8 getDecimals() {
    return decimals_;
  }

  __always_inline uint256 getRootKey() {
    return root_public_key_;
  }

  __always_inline TokensType getTotalSupply() {
    return total_supply_;
  }

  __always_inline TokensType getTotalGranted() {
    return total_granted_;
  }

  __always_inline cell getWalletCode() {
    return wallet_code_;
  }

  __always_inline TokenId getLastMintedToken() {
    return total_supply_;
  }

  __always_inline
  lazy<MsgAddressInt> getWalletAddress(int8 workchain_id, uint256 pubkey,uint256 own_addr) {
    return calc_wallet_init(workchain_id, pubkey,own_addr).second;
  }

  __always_inline
  void regTokenToExchange(uint256 exchange_publickey,address exchange_address,uint256 own_addr) {
    tvm_accept();
    //1.get stander TON address from adress hex
    auto workchain_id = std::get<addr_std>(address{tvm_myaddr()}.val()).workchain_id;
    // std::optional<address> exchange_addr;
    // if (exchange_address)
    //address exchange_addr = address::make_std(workchain_id, exchange_address);

    //2.get target wallet address from the exchange_address,all exchange's wallet should work in internal transfer.
    address token_wallet_address=calc_wallet_init(workchain_id, exchange_publickey,own_addr).second;
    uint256 token_wallet_address_hex_=std::get<addr_std>(token_wallet_address()).address;
    
    //3.call exchange function update the token into exchange.
    //exchange_addr_=token_wallet_address_hex_;
    handle<ITonExchange> dest_exchange(exchange_address);
    dest_exchange(Grams(0), SEND_REST_GAS_FROM_INCOMING).regNewToken(token_wallet_address_hex_,symbol_);

    
  }


  //--------------------System function------------------------------------------

  // received bounced message back
  __always_inline static int _on_bounced(cell msg, slice msg_body) {
    tvm_accept();

    using Args = args_struct_t<&ITONTokenWallet::accept>;
    parser p(msg_body);
    require(p.ldi(32) == -1, error_code::wrong_bounced_header);
    auto [opt_hdr, =p] = parse_continue<abiv1::internal_msg_header>(p);
    require(opt_hdr && opt_hdr->function_id == id_v<&ITONTokenWallet::accept>,
            error_code::wrong_bounced_header);
    auto args = parse<Args>(p, error_code::wrong_bounced_args);
    auto bounced_id = args.tokenId;

    auto [hdr, persist] = load_persistent_data<IRootTokenContract, root_replay_protection_t, DRootTokenContract>();
    require(bounced_id > 0, error_code::wrong_bounced_args);
    require(bounced_id <= persist.total_supply_, error_code::wrong_bounced_args);
    require(persist.total_granted_ > 0, error_code::wrong_bounced_args);
    --persist.total_granted_;
    persist.tokens_.insert(bounced_id);
    save_persistent_data<IRootTokenContract, root_replay_protection_t>(hdr, persist);
    return 0;
  }
  // default processing of unknown messages
  __always_inline static int _fallback(cell msg, slice msg_body) {
    return 0;
  }

  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(IRootTokenContract, root_replay_protection_t)
private:
  __always_inline
  std::pair<StateInit, lazy<MsgAddressInt>> calc_wallet_init(int8 workchain_id, uint256 pubkey,uint256 owner_addr) {
    DTONTokenWallet wallet_data {
      name_, symbol_, decimals_,
      root_public_key_, pubkey,
      lazy<MsgAddressInt>{tvm_myaddr()}, wallet_code_, {}, {},{},owner_addr
    };
    auto [wallet_init, dest_addr] = prepare_wallet_state_init_and_addr(wallet_data);
    lazy<MsgAddressInt> dest{ MsgAddressInt{ addr_std { {}, {}, workchain_id, dest_addr } } };
    return { wallet_init, dest };
  }
};

DEFINE_JSON_ABI(IRootTokenContract, DRootTokenContract, ERootTokenContract);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS(RootTokenContract, IRootTokenContract, DRootTokenContract, ROOT_TIMESTAMP_DELAY)