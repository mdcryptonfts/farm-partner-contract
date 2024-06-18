#pragma once

void partners::receive_nft_transfer(name owner, name receiver, vector<uint64_t>& ids, std::string memo){
    // This function only exists to avoid throwing errors if our contract is notified about an NFT transfer
}

void partners::receive_token_transfer(name from, name to, eosio::asset quantity, std::string memo){
    
    const name tkcontract = get_first_receiver();

    if ( quantity.amount == 0 || from == _self || to != _self ) return;

    check( quantity.amount > 0, "must send a positive quantity" );
    check( quantity.amount <= MAX_ASSET_AMOUNT, "quantity too large" );  

    state s = state_s.get();

    if( memo == "farm payment" ){
        extended_symbol token_to_check = {quantity.symbol, tkcontract};
        check( std::find( s.accepted_tokens.begin(), s.accepted_tokens.end(), token_to_check ) != s.accepted_tokens.end(), "this token is not accepted" );

        asset price_on_waxdao = get_farm_price( _self, {quantity.symbol, tkcontract} );
        int64_t partner_fee = calculate_asset_share( price_on_waxdao.amount, s.partner_fee_1e6 );
        asset expected_total = price_on_waxdao + asset( partner_fee, quantity.symbol );
        check( quantity >= expected_total, ( "expected to receive " + expected_total.to_string() ).c_str() );

        add_point( from );
        transfer_tokens( WAXDAO_CONTRACT, price_on_waxdao, tkcontract, std::string("farm payment") );
        return;
    }

    // Any memos caught below here should be dynamic, consisting of multiple parts
    std::vector<std::string> memo_parts = parse_memo( memo );

    if( memo_parts[1] == "rewards" ){
        check( memo_parts.size() >= 4, "memo for rewards deposit operation is incomplete" );
        const eosio::name farm_name = eosio::name( memo_parts[2] );
        uint64_t reward_id = std::strtoull( memo_parts[3].c_str(), NULL, 0 );

        farm_struct farm = get_farm( farm_name );
        check( from == farm.creator, "only the farm creator can deposit rewards" );
        transfer_tokens( WAXDAO_CONTRACT, quantity, tkcontract, memo );

        return;
    }

    if( memo_parts[1] == "withdraw" ){
        check( from == WAXDAO_CONTRACT, "expected waxdao to be the sender" );
        check( memo_parts.size() >= 4, "memo for withdraw operation is incomplete" );
        const eosio::name farm_name = eosio::name( memo_parts[2] );

        farm_struct farm = get_farm( farm_name );
        transfer_tokens( farm.creator, quantity, tkcontract, memo );

        return;
    }    

}