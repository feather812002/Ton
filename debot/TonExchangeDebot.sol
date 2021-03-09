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
     function getAllSupportTokens() external view returns (SupportToken[] supportTokens);
}

struct SupportToken {
  uint256 exchange_wallet_addr;
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
        Sdk.getBalance(tvm.functionId(setBalance), value);
        Sdk.getAccountType(tvm.functionId(getAllSupportTokens), value);
        exchange_address = value;
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
            (uint64 dec, uint64 float) = tokens(m_balance);
            Terminal.print(0, format("Account balance: {}.{}", dec, float));
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
            Terminal.print(0, "This is start  get to all support tokens of this exchange");
            Terminal.print(0, format("Exchange address :{} ",exchange_address));
           
        }
        

         
    }

    function getAllSupportTokensError() public{
          Terminal.print(0, "This is wrong when get to all support tokens");
    }

    function showAllSupportTokens(SupportToken[] supportTokens) public accept functionID(0x10){
     
        Terminal.print(0,format("Total support tokens: {} ",supportTokens.length));
        
        support_token_list = new SupportToken[](supportTokens.length);
        SupportToken newSupportToen=supportTokens[1];
        // newSupportToen.exchange_wallet_addr=uint256(10);
        // newSupportToen.token_symbol="0";
         Terminal.print(0, format("{}",newSupportToen.exchange_wallet_addr));
	
        for (uint32 i = 0; i < supportTokens.length; i++) {
            
            Terminal.print(0, format("{}",uint256(i)));
		}
        // for (uint8 i = 0; i < support_token_list.length; i++) {
        //      support_token supporttoken=support_token_list[i];
        //      Terminal.print(0, format("{}. {}",i,supporttoken.exchange_wallet_addr));
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