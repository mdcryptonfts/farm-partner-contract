# Token Farm Partner Contract

The smart contract files are in the `/contracts` directory. The compiled versions are in the `build` directory, you can just upload them to your wallet on waxblock if you don't want to compile them yourself.

## How It Works

If you are building your own front end, you probably want to accept tokens via your own contract. This will allow you to collect additional fees on top of what WaxDAO collects from you via farm payments.

This contract acts as an intermediary. Instead of having your users send payment directly to WaxDAO, you are able to have them send payments to your contract. 

The partner contract will calculate the current price, based on your partner discount and the payment token. It will only send that amount to WaxDAO, and the rest will stay in your contract. Optionally, you can also redirect collected fees to another wallet.

When users create farms and reward pools, they will also do this by interacting with your partner contract. The contract will call inline actions which then submit the necessary transactions to the `tf.waxdao` contract.

When staking, claiming rewards etc, users will interact directly with the `tf.waxdao` contract.

## Getting Started

Once you have the contract deployed, here are the steps you need to take.

1. `init` action

This action will initialize the `state` singleton, which stores a list of tokens you are willing to accept for farm creation, and a percentage fee, which can be set between 0 and 100% on top of the standard farm price.

By default, the init action will set the tokens list to accept WAX only, and set the percentage fee at 10%. However, you can adjust these using contract actions which will be explained in the steps below.

2. `setpaymethod` action

This allows you to add additional tokens as payment methods. Keep in mind that these tokens must be listed as an accepted token on the `tf.waxdao` contract if you plan on paying for farms with them.

You can request a token listing by reaching out to us on Telegram. Keep in mind that we will only add well known tokens that have sufficient liquidity.

The `setpaymethod` action takes an `extended_symbol`, which is formatted as shown below.

`{sym: "8,WAX", contract: "eosio.token"}`

3. `setfee` action

This allows you to adjust the percentage fee that you want to charge, in addition to the standard farm price. It is a `uint64_t`, and is scaled by 1e6.

For example, if a farm costs 150 WAX after your partner discount, and you see the fee to 50%, users will send 225 WAX to your contract, netting you 75 WAX in profit.

To format 50% as a 1e6 scaled integer, you would do it like this.

`partner_fee_1e6: 50000000`  (50 with 6 zeroes after)

*Steps 4 and 5 should be packaged together into a single transaction*

4. `farm payment` memo

Once your contract is configured with the settings you prefer, users can pay for farms by sending an accepted token to your contract with the memo: `farm payment`

When your contract receives the tokens, it will calculate the amount to send to WaxDAO, and automatically make the transfer, leaving the remainder (if there is any) for you as profit.

5. `createfarm` action

After a user has paid for their farm, they will be credited with 1 "farm point" which is stored in the `points` table on your contract.

They can now call the `createfarm` action, where they will submit the details for the staking token (action details can be found in `tokenstaking.entry.cpp`)

When this action is called, your contract will check to make sure they have a point. Then debit their point from them, and finally create an inline action which calls the `createfarm` action on the `tf.waxdao` contract.

The farm is now created, and the staking details are stored in the `tf.waxdao` contract. A reward pool (or up to 10 reward pools) still needs to be created.

*Steps 6 and 7 should be combined into a single transaction. Ideally, you can actually bundle steps 4, 5, 6 and 7 into 1 transaction*

6. `addreward` action

This is the action where a user will submit the details for the reward token and the length of the reward period. More information about the action params and behavior can be found in `tokenstaking.entry.cpp`

This action will also create an inline action, which calls the `addreward` action on the `tf.waxdao` contract.

7. `|rewards|<farm_name>|<reward_id>|` memo

Once the reward pool is created, the farm owner can deposit rewards. Since your contract is acting as a middleman, you are technically the farm creator.

For this reason, you need to be able to differentiate between which farms belong to which users. This is why the `farms` table exists on the partner contract. It keeps a list of farms that you created, along with the wallet addresses of the users who paid for them.

When a user sends this memo to your contract, it will check the `farms` table to make sure they are the farm creator. Then, an inline action will be called which redirects the reward deposit to `tf.waxdao`

The farm name and ID of the reward pool need to be included in the memo. In order to know beforehand what the ID of the next reward pool will be, you can check the `global` table on the `tf.waxdao` contract before you submit the `addreward` transaction.

The value of `total_incentives_created` in the global table is the id of the next reward pool. For example, if `total_incentives_created` was `0`, the next reward ID would be `0`. So always use the current value stored in the table.

If the name of a farm was `myfarm`, and `total_incentives_created` was 5, you would submit your memo like this:

`|rewards|myfarm|5|`

--- 

At this point, the farm is all set up and tokens can be staked to it (assuming that the creator set the reward period to start instantly).

Documentation for the `tf.waxdao` contract, including which actions need to be called for staking/claiming/unstaking etc, can be found [in our Gitbook](https://waxdao.gitbook.io/waxdao)