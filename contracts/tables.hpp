#pragma once

struct[[eosio::table]] stat {
  eosio::asset supply;
  eosio::asset max_supply;
  eosio::name issuer;

  uint64_t primary_key() const { return supply.symbol.code().raw(); }
};
typedef eosio::multi_index<"stat" _n, stat> stat_table;

struct[[ eosio::table, eosio::contract(CONTRACT_NAME) ]] farms {
  eosio::name farm_name;
  eosio::name creator;

  uint64_t primary_key() const { return farm_name.value; }
};
using farms_table = eosio::multi_index<"farms" _n, farms>;

struct[[ eosio::table, eosio::contract(CONTRACT_NAME) ]] points {
  eosio::name wallet;
  uint8_t points_balance;

  uint64_t primary_key() const { return wallet.value; }
};
using points_table = eosio::multi_index<"points" _n, points>;

struct[[ eosio::table, eosio::contract(CONTRACT_NAME) ]] state {
  vector<eosio::extended_symbol> accepted_tokens;
  uint64_t partner_fee_1e6;

  EOSLIB_SERIALIZE(state, (accepted_tokens)(partner_fee_1e6))
};
using state_singleton = eosio::singleton<"state" _n, state>;