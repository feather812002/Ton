pragma ton-solidity >= 0.35.0;
pragma AbiHeader expire;
pragma AbiHeader time;
pragma AbiHeader pubkey;
// pragma directive<smt_checker>;
pragma experimental SMTChecker;
import "./sdk/Debot.sol";
import "./sdk/Terminal.sol";
import "./sdk/AddressInput.sol";
import "./sdk/Sdk.sol";
import "./sdk/Menu.sol";

interface ITonExchange{
    //function getMyTakerOrders(uint256 taker_address) external returns(Order[] value0); 
    function getSupportTokenByRoot(uint256 root_addr_hex) external returns(uint256 exchange_wallet_addr,bytes token_symbol); 
    function getRootAddress() external  returns (uint256 value0);
    function getAllOrdersSize() external  returns (uint32 value0);
     function getAllSupportTokens() external  returns (SupportToken[] value0);
     function getSupportTokenByNo(uint128 token_count)  external  returns (uint256 token_root_addr,bytes token_symbol);
     function getNFFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) external pure  returns (uint128[] value0);
     function getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) external  returns (bytes token_name,bytes token_symbol,uint8 decimals,uint128 token_balance);
}

interface ITONTokenWallet{
    function getRootAddress() external pure functionID(0x16) returns (address value0);
}

struct SupportToken {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
}

struct Order {
  uint32  order_no;
  uint256 sell_token_root_address_hex;
  uint128 sell_token_amount;
  uint256 seller_send_token_address_hex;
  uint256 seller_resive_token_address_hex;
  bytes   sell_token_symbol;
  uint8   sell_token_type;
  uint256 buy_token_root_address_hex;
 uint128 buy_token_amount;
  uint256 buyer_send_token_address_hex;
  uint256 buyer_resive_token_address_hex;
  bytes   buy_token_symbol;
  uint8   buy_token_type;
  //0:expired,1:put, 2:full filled,3:part filled,4:cancle. 
  uint8 order_status;
  uint64  expired;  
}

struct ShowSupportToken {
  uint256 token_root_addr;
  bytes token_symbol;
}

struct customer_token {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  uint8 decimals;
  uint128 token_balance;
}

struct customer_nftoken {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  uint128[]  tokenid_list;
}
contract TonExchangeDebot is Debot {

    uint128 m_balance;

    modifier onlyOwnerAccept() {
        require(tvm.pubkey() == msg.pubkey(), 100);
        tvm.accept();
        _;
    }
    modifier accept() {
        tvm.accept();
        _;
    }
    address exchange_address;
    address wallet_address;
    address wallet_root_address;
    uint128[]  nf_token_exchange_balance;
    uint128 supportTokenCount ;
    customer_token f_token_exchange_balance;


    SupportToken[] support_token_list;

    constructor(string debotAbi) public {
        require(tvm.pubkey() == msg.pubkey(), 100);
        tvm.accept();
       // exchange_address=exchange_address_;
        init(DEBOT_ABI, debotAbi, "", address(0));
    }
    function fetch() public override returns (Context[] contexts) {}


    function setTargetABI(string tabi) public onlyOwnerAccept {
        m_targetAbi.set(tabi);
        m_options |= DEBOT_TARGET_ABI;


    }

    function start() public override {
        Menu.select("Main menu", "Hello, This is Ton exchange debot", [
            MenuItem("Select exchange", "", tvm.functionId(selectExchange)),
            MenuItem("Exit", "", 0)
        ]);
    }

     function selectExchange(uint32 index) public {
        index = index;
        Terminal.print(0, "Please, enter exchange address:");
        AddressInput.select(tvm.functionId(checkExchange));
    }

    function checkExchange(address value) public {
        //Sdk.getBalance(tvm.functionId(setBalance), value);
        Sdk.getAccountType(tvm.functionId(getAllSupportTokens), value);
        exchange_address = value;
        
        Terminal.print(0, format("Exchange address :{} ",exchange_address));
	}
     function setBalance(uint128 nanotokens) public {
        m_balance = nanotokens;
    }

    function setExchange() public {
        Terminal.print(0, "Please, enter the exchange address:");
       // AddressInput.select(tvm.functionId(checkWallet));
    }

    function quit() public override {

    }

    function getExchangeAddress() public view returns(address){
        return exchange_address;
    }


    function getVersion() public override returns (string name, uint24 semver) {
        (name, semver) = ("Ton Exchange", 4 << 8);
    }

    function getAllSupportTokens(int8 acc_type) public  {
      //  index = index;
         if (acc_type == -1)  {
            Terminal.print(0, format("Account with exchange address {} doesn't exist", exchange_address));
        } else {
            string state = "";
            if (acc_type == 0) {
                state = "Uninit";
            } else if (acc_type == 2) {
                state = "Frozen";
            } else if (acc_type == 1) {
                state = "Active";
            }
            Terminal.print(0, "Exchange Account state: " + state);
            if (state != "Active") {
                return;
            }
            uint32 expiretime = now + 60000;
            optional(uint256) pubkey;
            ITonExchange(exchange_address).getAllSupportTokens{
                extMsg: true,
                time: uint64(now),
                sign: false,
                pubkey: pubkey,
                callbackId:0x10,
                onErrorId:0,
                abiVer:2,
                expire: expiretime
            }();
           
           
        }
        

         
    }

    function printMain() public {
        Menu.select("Main menu", "Hello, This is Ton exchange debot", [
            MenuItem("Add a new token", "", tvm.functionId(showSupportToken)),
            MenuItem("List all orders", "", tvm.functionId(showSupportToken)),
            MenuItem("Orders manager", "", tvm.functionId(printOrder)),
            MenuItem("Exchange Account Balance", "", tvm.functionId(printBalance)),
            MenuItem("Exit", "", 0)
        ]);
    }

    function printBalance(uint32 index) public {
        index=index;
        Menu.select("Exchange Account Balance", "", [
            MenuItem("My Balance ", "", tvm.functionId(inputWallet)),
            MenuItem("Deposit ", "", tvm.functionId(showSupportToken)),
            MenuItem("Withdraw", "", tvm.functionId(showSupportToken)),
            MenuItem("Back to main", "", tvm.functionId(printMain))
        ]);
    }

    function printOrder() public {
         Menu.select("Order", "", [
            MenuItem("Put order ", "", tvm.functionId(showSupportToken)),
            MenuItem("Cancle order", "", tvm.functionId(showSupportToken)),
            MenuItem("Fill order", "", tvm.functionId(printMain)),
            MenuItem("Back to main", "", tvm.functionId(printMain))
        ]);

    }


    
    

    function getAllSupportTokensError() public{
          Terminal.print(0, "This is wrong when get to all support tokens");
    }

    function getAllSupportTokensLength(SupportToken[] value0) public accept functionID(0x10){
        support_token_list=value0;
        Terminal.print(0,format("Total support tokens: {} ",support_token_list.length));
       
        uint32 expiretime = now + 60000;
        optional(uint256) pubkey;
        
        for (uint8 i = 0; i < support_token_list.length; i++) {
            
             //support_token supporttoken=support_token_list[i];
             ITonExchange(exchange_address).getSupportTokenByNo{
                extMsg: true,
                time: uint64(now),
                sign: false,
                pubkey: pubkey,
                callbackId:tvm.functionId(showSupportToken),
                onErrorId:0,
                abiVer:2,
                expire: expiretime
            }(uint128(i));
        }
       
         
    }

    function showSupportToken(uint256 token_root_addr,bytes token_symbol) public  {
        Terminal.print(0, format("{} : 0x{}",token_symbol,token_root_addr));
        supportTokenCount++;
        if(supportTokenCount==support_token_list.length){
             Terminal.print(0, "---------------------------------------------");
          
             printMain();
             //
        }
    }

    function inputWallet(uint32 index ) public {
        index=index;
        Terminal.print(0, "---------------------------------------------");
         
        Terminal.print(0, "Please, enter your wallet address:");
        AddressInput.select(tvm.functionId(checkWallet));
    }

    function checkWallet(address value) public {
        //Sdk.getBalance(tvm.functionId(setBalance), value);
       
        wallet_address = value;
        /// @notice Explain to an end user what this does
        /// @dev Explain to a developer any extra details
        /// @return Documents the return variables of a contract’s function state variable
        /// @inheritdoc	Copies all missing tags from the base function (must be followed by the contract name)Terminal.print(0, format("You wallet address :{} ",wallet_address));
        Sdk.getAccountType(tvm.functionId(getWalletRoot), value);
	}

    function getWalletRoot(int8 acc_type) public {
         if (acc_type == -1)  {
            Terminal.print(0, format("Account with wallet address {} doesn't exist", exchange_address));
        } else {
            string state = "";
            if (acc_type == 0) {
                state = "Uninit";
            } else if (acc_type == 2) {
                state = "Frozen";
            } else if (acc_type == 1) {
                state = "Active";
            }
            Terminal.print(0, "Wallet account state: " + state);
            if (state != "Active") {
                return;
            }
            uint32 expiretime = now + 60000;
             optional(uint256) pubkey;
            ITONTokenWallet(wallet_address).getRootAddress{
                extMsg: true,
                time: uint64(now),
                sign: false,
                pubkey: pubkey,
                callbackId:tvm.functionId(setWalletRootAddress),
                onErrorId:0,
                abiVer:2,
                expire: expiretime
            }();
        }
    }


    function setWalletRootAddress(address value0) public accept {
        
        wallet_root_address=value0;
        Terminal.print(0,format("setWalletRootAddress root:{}",wallet_root_address));
        Terminal.print(0,format("Exchange address:{}",exchange_address));
        Terminal.print(0, "---------------------------------------------");
        Menu.select("My Balance", "", [
            MenuItem("Get token balance of the exchange ", "", tvm.functionId(getWalletBalanceByWallet)),
           
            MenuItem("Back to main", "", tvm.functionId(printMain))
        ]);

        
    }
    function getWalletBalanceByWallet(uint32 index) public accept view {
        index=index;
        uint32 expiretime = now + 60000;
        optional(uint256) pubkey;
        int8 wid;
        uint256 root_hex;
        uint256 wallet_address_hex;
        bool status;
        uint hexss;
        (wid,root_hex)=wallet_root_address.unpack();
        (wid,wallet_address_hex)=wallet_address.unpack();
        (hexss, status) = stoi("0x35f05f37bfe1f05ea9737c855bfa629eba33ccb95f0fd208ee56052fe62159c1");
        // TvmCell message;
        // message=tvm.buildExtMsg({abiVer: 2, callbackId: tvm.functionId(setNFBalance), onErrorId: 0, dest: exchange_address, time: uint64(now),
        //  expire: expiretime, call: {ITonExchange.getFungibleTokenBalance, wallet_address_hex,root_hex}, pubkey: pubkey, sign: true});
        // tvm.sendrawmsg(message, 1);
        ITonExchange(exchange_address).getNFFungibleTokenBalance{
                extMsg: true,
                time: uint64(now),
                sign: false,
                pubkey: pubkey,
                callbackId:tvm.functionId(setNFBalance),
                onErrorId:0,
                abiVer:2,
                expire: expiretime
        }(wallet_address_hex,root_hex);
    }

    function getSupportTokenByRootreturn(uint256 exchange_wallet_addr,bytes token_symbol) public accept {
         Terminal.print(0,format("getSupportTokenByRootreturn :{}-{}",exchange_wallet_addr,token_symbol));
    }
    function getExchangeRoot(uint256 value0) public accept {
        Terminal.print(0,format("Exchange root:{}",value0));
    }
    function getOrdersSize(uint32 value0) public accept {
        Terminal.print(0,format("Total order:{}",value0));
    }


    function setNFBalance(uint128[] value0) public accept{
        Terminal.print(0,format("setNFBalance work:{}",value0.length));
        Terminal.print(0,format("customer address:{}",wallet_address.value));
        Terminal.print(0,format("customer root address:{}",wallet_root_address.value));
        uint256 wallet_address_hex=uint256(wallet_address.value);
        uint256 wallet_address_root_hex=uint256(wallet_root_address.value);
       
        nf_token_exchange_balance=value0;
        if(nf_token_exchange_balance.length==0){
            uint32 expiretime = now + 60000;
            optional(uint256) pubkey;
            ITonExchange(exchange_address).getFungibleTokenBalance{
                extMsg: true,
                time: uint64(now),
                sign: false,
                pubkey: pubkey,
                callbackId:tvm.functionId(setFTBalance),
                onErrorId:0,
                abiVer:2,
                expire: expiretime
            }(wallet_address_hex,wallet_address_root_hex);
        }else{
            string str="";
            for(uint128 tokenid:nf_token_exchange_balance){
                str.append(format("{},",tokenid));
            }
            Terminal.print(0, format("NFtoken balance: {} " , str));
        }

    }

    function setFTBalance(bytes token_name,bytes token_symbol,uint8 decimals,uint128 token_balance) public accept{
        f_token_exchange_balance.token_name=token_name;
        f_token_exchange_balance.token_symbol=token_symbol;
        f_token_exchange_balance.decimals=decimals;
        f_token_exchange_balance.token_balance=token_balance;

        Terminal.print(0, format("Figible token :{} , symbol :{} ,balance : {}" ,token_name,token_symbol,token_balance));
        
    }

  

    function addNewToken() public {
    }


    function tokens(uint128 nanotokens) private pure returns (uint64, uint64) {
        uint64 decimal = uint64(nanotokens / 1e9);
        uint64 float = uint64(nanotokens - (decimal * 1e9));
        return (decimal, float);
    }


}