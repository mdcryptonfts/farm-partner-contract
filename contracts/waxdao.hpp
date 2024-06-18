#pragma once

using namespace std;

namespace waxdao {

struct[[eosio::table]] global {
  uint64_t maximum_lock_seconds;
  uint64_t minimum_lock_seconds;
  uint64_t total_farms_created;
  uint64_t total_incentives_created;
  uint8_t maximum_incentives_per_farm;
  uint64_t minimum_reward_duration;
  uint64_t maximum_reward_duration;
  eosio::asset farm_price_in_wax;
  vector<eosio::name> admin_wallets;
  eosio::name fee_collection_wallet;
  uint32_t observation_seconds;

  EOSLIB_SERIALIZE(
      global,
      (maximum_lock_seconds)(minimum_lock_seconds)(total_farms_created)(
          total_incentives_created)(maximum_incentives_per_farm)(
          minimum_reward_duration)(maximum_reward_duration)(farm_price_in_wax)(
          admin_wallets)(fee_collection_wallet)(observation_seconds))
};
using global_singleton = eosio::singleton<"global"_n, global>;

struct[[eosio::table]] partners {
  eosio::name wallet;
  uint64_t discount_1e6;

  uint64_t primary_key() const { return wallet.value; }
};
using partners_table = eosio::multi_index<"partners"_n, partners>;

struct[[eosio::table]] paymethods {
  uint64_t alcor_pool_id;
  eosio::extended_symbol token_symbol;
  uint64_t discount_1e6;

  uint64_t primary_key() const { return alcor_pool_id; }
  uint128_t by_sym_contract() const {
    return mix64to128(token_symbol.get_symbol().code().raw(),
                      token_symbol.get_contract().value);
  }
};
using paymethods_table = eosio::multi_index<
    "paymethods"_n, paymethods,
    eosio::indexed_by<"symcontract"_n,
                      eosio::const_mem_fun<paymethods, uint128_t,
                                           &paymethods::by_sym_contract>>>;
}