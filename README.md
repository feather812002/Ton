# TonExchange Introduction

TonExchange is a DEX trade system , it  work with Order Book mode. it already support follow function :

* User can add new TIP3 token support (fungible and non fungible) by themself.
* User can deposit or withdrawal TIP3 token to exchange.
* User can put ,cancel and fill order with already support TIP3 tokens.
* Exchange support any fungible token or non fungible token pair . (FT:FT,NFT:FT, NFT:NFT).
* TonExchange will make sure the order trade success or fail in all.It can ensure that the transmission of both parties in the transaction constitutes an atomic transaction mode.
make sure one order will  succeed at the same time, or fail at the same time.
* TonExchange security control by smart contract, so it is enough security for a exchange .
* TonExchange support charge fee.
  The maker will 0 fee and only need pay gas fee for put order.
  The taker will pay 1 Ton for each fill order , the 0.5 ton will transfer to maker address as encourage fluidity.
  (because we support NFT:FT tarde pairs , so the fee charge by trade token itself is difficult and impossible, so for now ,I simple let the fee work with Ton)





# TonExchange Install

## 1. TIP3 improve and some limit 

* Improve the (approve) and (allowance) function to support multiple address for fungible and  non-fungible tokens. 

* Add some exchange function , let customer can work with exchange from the wallet.

* Add deployEmptyWallet to  non-fungible  Root contract and exposed the deployEmptyWallet from fugible root contract(I don't know why the TIP3 source code hide the deployEmptyWallet to internal call).



## 2. Deploy TIP3 token root contract

Deploy TIP3 fungible and nonfugible wotk follow this url:

https://github.com/tonlabs/ton-labs-contracts/tree/master/cpp/tokens-fungible
The fungible and nonfugilbe work .

> Note: It only tested with Ton SE node, it maybe get hang when you want deploy it net.ton.dev, it look like the a large string as input parameter in the deploy will cause hang and always get the 507 error return.

## 3. Deploy Wallet contract

Call  deployEmptyWallet or deployWallet method from root contract to deploy wallet contract.

* deployEmptyWallet can be call by any one from client or other contract in the source design , but in my test it only work with external call.

* deployWallet only work with root contract owner at now .

> Note: Don't deploy the TIP3 wallet by tonos-cli , only deploy it by the deployEmptyWallet or deployWallet function from the root contract.because ,According to TIP3's address calculation mechanism , when it try to get the expected address ,it must need the initial parameters input ,for example : token name, token symbol,token decimals etc. but when you work with tonos-cli command ,the command should be(/tonos-cli genaddr -v --genkey ${.key.json} {.tvc} ${.abi.json}) , it must can't pass these initial parameters, so in last you will get a different address, then you can success deploy the wallet contract to online ,but must can't send token to other one .

## 4. Deploy TonExchange contract 
You can call by ./deploy.sh script deploy TonExchange contract.
But before you start deploy the TonExchange contract you should setting some environment variable:

```
export CLI_PATH=./tonos-cli/target/release
export DEPLOY_LOCAL="1"
export LOCAL_GIVER_PATH=../
export TVM_INCLUDE_PATH=../../TON-Compiler/llvm/projects/ton-compiler/cpp-sdk
```
Make sure above these path your local path.