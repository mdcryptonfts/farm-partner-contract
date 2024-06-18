#pragma once

#define CONTRACT_NAME "partners"
#define mix64to128(a, b) (uint128_t(a) << 64 | uint128_t(b))   // Credit to Block Chain Spock

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/symbol.hpp>
#include <eosio/action.hpp>
#include <eosio/crypto.hpp>
#include <eosio/singleton.hpp>
#include <eosio/transaction.hpp>
#include <alcorpriceoracle_interface.hpp>
#include <cmath>
#include "safecast.hpp"
#include "constants.hpp"
#include "waxdao.hpp"
#include "tables.hpp"
#include "structs.hpp"

using namespace eosio;
using namespace std;


CONTRACT partners : public contract {
    public:
        using contract::contract;
        partners(name receiver, name code, datastream<const char *> ds):
        contract(receiver, code, ds),
        global_s(WAXDAO_CONTRACT, WAXDAO_CONTRACT.value),
        state_s(receiver, receiver.value)
        {}

        // Main Actions
        ACTION addreward(const name& farm_name, const bool& start_now, const uint64_t& start_time, const uint64_t& duration,
            const extended_symbol& reward_token);
        ACTION createfarm(const name& creator, const name& farm_name, const extended_symbol& staking_token,
            const uint64_t& vesting_time);
        ACTION extendreward(const name& farm_name, const uint64_t& reward_id, const bool& start_now, const uint64_t& start_time, const uint64_t& duration);
        ACTION init();
        ACTION rempaymethod(const extended_symbol& payment_method);
        ACTION setfee(const uint64_t& partner_fee_1e6);
        ACTION setpaymethod(const extended_symbol& payment_method);
        ACTION withdraw(const name& farm_name);


        // Notifications
        [[eosio::on_notify("atomicassets::transfer")]] void receive_nft_transfer(name owner, name receiver, vector<uint64_t>& ids, std::string memo);
        [[eosio::on_notify("*::transfer")]] void receive_token_transfer(name from, name to, eosio::asset quantity, std::string memo);

    private:

        // Singletons
        waxdao::global_singleton global_s;
        state_singleton state_s;

        // Tables
        farms_table farms_t = farms_table( _self, _self.value );
        waxdao::partners_table partners_t = waxdao::partners_table( WAXDAO_CONTRACT, WAXDAO_CONTRACT.value );
        waxdao::paymethods_table payments_t = waxdao::paymethods_table( WAXDAO_CONTRACT, WAXDAO_CONTRACT.value );
        points_table points_t = points_table( _self, _self.value );

        // Functions
        inline eosio::permission_level active_perm();
        void add_point(const name& user);
        int64_t calculate_asset_share(const int64_t& quantity, const uint64_t& percentage);
        int64_t get_expected_quantity(waxdao::global& g, const uint64_t& pool_id, const int64_t& expected_quantity);
        inline farm_struct get_farm(const name& farm_name);
        asset get_farm_price(const name& partner, const extended_symbol& payment_token);
        bool is_wax(const name& contract, const asset& quantity);
        int64_t mulDiv(const uint64_t& a, const uint64_t& b, const uint128_t& denominator);
        std::vector<std::string> parse_memo(std::string memo);
        void remove_point(const name& user);
        bool token_exists(const extended_symbol& token);
        void transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo);
    
};



