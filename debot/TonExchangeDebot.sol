pragma ton-solidity >= 0.35.0;
pragma AbiHeader expire;
pragma AbiHeader time;
pragma AbiHeader pubkey;
import "./sdk/Debot.sol";
import "./sdk/Terminal.sol";
import "./sdk/AddressInput.sol";
import "./sdk/Sdk.sol";
import "./sdk/Menu.sol";

interface ITonExchange{
     function getAllSupportTokens() external  returns (SupportToken[] value0);
     function getSupportTokenByNo(uint128 token_count)  external  returns (uint256 token_root_addr,bytes token_symbol);
}
struct SupportToken {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
}

struct ShowSupportToken {
  uint256 token_root_addr;
  bytes token_symbol;
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
        //Sdk.getAccountType(tvm.functionId(getAllSupportTokens), value);
        exchange_address = value;
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
        Terminal.print(0, "This is start  get to all support tokens of this exchange");
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
            Terminal.print(0, format("Account with address {} doesn't exist", exchange_address));
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
           
           
        }
        

         
    }

    function printMain() public {
        Menu.select("Main menu", "Hello, This is Ton exchange debot", [
            MenuItem("List all orders", "", tvm.functionId(showSupportToken)),
            MenuItem("List all orders", "", tvm.functionId(showSupportToken)),
            MenuItem("Exit", "", 0)
        ]);
    }
    

    function getAllSupportTokensError() public{
          Terminal.print(0, "This is wrong when get to all support tokens");
    }

    function getAllSupportTokensLength(SupportToken[] value0) public accept functionID(0x10){
        
        Terminal.print(0,format("Total support tokens: {} ",value0[1].exchange_wallet_addr));
        support_token_list=value0;
        // uint32 expiretime = now + 60000;
        // optional(uint256) pubkey;
        //printMain();
        // for (uint8 i = 0; i < support_token_list.length; i++) {
        //      //support_token supporttoken=support_token_list[i];
        //      ITonExchange(exchange_address).getSupportTokenByNo{
        //         extMsg: true,
        //         time: uint64(now),
        //         sign: false,
        //         pubkey: pubkey,
        //         callbackId:0x11,
        //         onErrorId:0,
        //         abiVer:2,
        //         expire: expiretime
        //     }(uint128(i));
        // }
         
    }

    function showSupportToken(uint256 token_root_addr,bytes token_symbol) public accept functionID(0x11){
        Terminal.print(0, format("0x{:x} : {:x}",token_root_addr,token_symbol));
       
        // for (uint32 i = 0; i < support_token_list.length; i++) {
        //    //Terminal.print(0, format("{}. {:x},{}",i,support_token_list[i].exchange_wallet_addr,i));
		// }
    }

    function addNewToken() public {
    }


    function tokens(uint128 nanotokens) private pure returns (uint64, uint64) {
        uint64 decimal = uint64(nanotokens / 1e9);
        uint64 float = uint64(nanotokens - (decimal * 1e9));
        return (decimal, float);
    }


}