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
     function getAllSupportTokens() external returns (support_token[] supportTokens);
}

struct support_token {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
}
contract TonExchangeDebot is Debot {

    modifier onlyOwnerAccept() {
        require(tvm.pubkey() == msg.pubkey(), 100);
        tvm.accept();
        _;
    }
    address exchange_address;
    TvmCell sendmessage;

    constructor(address exchange_address_,string debotAbi) public {
        require(tvm.pubkey() == msg.pubkey(), 100);
        tvm.accept();
        exchange_address=exchange_address_;
        init(DEBOT_ABI, debotAbi, "", address(0));
    }
    function fetch() public override returns (Context[] contexts) {}


    function setTargetABI(string tabi) public onlyOwnerAccept {
        m_targetAbi.set(tabi);
        m_options |= DEBOT_TARGET_ABI;
        m_target.set(exchange_address);
        m_options |= DEBOT_TARGET_ADDR;

    }

    function start() public override {
        Menu.select("Main menu", "Hello, This is Ton exchange debot", [
            MenuItem("0.Config exchange address.", "", tvm.functionId(setExchange)),
            MenuItem("1.List all support token.", "", tvm.functionId(getAllSupportTokens)),
            MenuItem("2.Reg a new token.", "", tvm.functionId(addNewToken)),
            MenuItem("3.set Exchange.", "", tvm.functionId(setExchange)),
            MenuItem("Exit", "", 0)
        ]);
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

    function getAllSupportTokens(uint32 index) public  {
        index = index;
        optional(uint256) pubkey;
        Terminal.print(0, "This is start  get to all support tokens");
        //uint32 expiretime = now + 60000;
        ITonExchange(exchange_address).getAllSupportTokens{
            extMsg: true,
            time: uint64(now),
            sign: false,
            pubkey: pubkey,
            callbackId:tvm.functionId(showAllSupportTokens),
            onErrorId:tvm.functionId(getAllSupportTokensError),
            abiVer:2,
            expire: tvm.functionId(getAllSupportTokensError)
        }();
        // uint32 expiretime = now + 60000;
        // sendmessage = tvm.buildExtMsg({abiVer: 2, callbackId: tvm.functionId(showAllSupportTokens), onErrorId: tvm.functionId(getAllSupportTokensError), dest: exchange_address, 
        // time: now, expire: expiretime, call: {ITonExchange.getAllSupportTokens}, pubkey: pubkey, sign: false});
        // tvm.sendrawmsg(sendmessage,1);
         
    }

    function getAllSupportTokensError() public{
          Terminal.print(0, "This is wrong when get to all support tokens");
    }

    function showAllSupportTokens(support_token[] tokens) public {
        uint8 count=uint8(0);
        Terminal.print(0, "This is success test when get to all support tokens");
        for(support_token token:tokens){
              Terminal.print(0, format("{} .{}:{} ", count,token.exchange_wallet_addr,token.token_symbol));
              count++;
        }
         
    }

    function addNewToken() public {
    }


}