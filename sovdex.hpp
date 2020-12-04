#pragma once

#include <eosio/asset.hpp>

namespace sovdex {

    using eosio::asset;
    using eosio::symbol;
    using eosio::name;

    const name id = "sovdex"_n;
    const name code = "sovdexrelays"_n;
    const string description = "SovDex Swap Converter";

    const extended_symbol SOV  { symbol{"SOV",4}, "sovmintofeos"_n };
    const extended_symbol SVX  { symbol{"SVX",4}, "svxmintofeos"_n };
    /**
     * SovDex markets
     */
    struct [[eosio::table]] pair_row {
        string          pair;
        asset           connectorbal;
        asset           outstandingbal;
        uint64_t        cw;
        bool            enabled;
        bool            reinvestfees;
        float_t         price;
        asset           adminfees;

        uint64_t primary_key() const { return connectorbal.symbol.code().raw(); }
    };
    typedef eosio::multi_index< "pair"_n, pair_row > sovtable;

    /**
     * ## STATIC `get_fee`
     *
     * Get sovdex total fee. 0.2%
     *
     * ### params
     *
     * ### returns
     *
     * - `{uint8_t}` - total fee (trade + protocol)
     *
     * ### example
     *
     * ```c++
     * const uint8_t fee = sovdex::get_fee();
     * // => 0
     * ```
     */
    static uint8_t get_fee()
    {
        return 20;
    }

    /**
     * ## STATIC `get_reserves`
     *
     * Get reserves for a pair
     *
     * ### params
     *
     * - `{symbol} sym_in` - symbol for reserve1
     * - `{symbol} sym_in` - symbol for reserve2
     *
     * ### returns
     *
     * - `{pair<asset, asset>}` - pair of reserve assets
     *```
     */
    static std::pair<uint64_t, uint64_t> get_reserves( symbol sym_in, symbol sym_out)
    {
        check(sym_in!=sym_out, "SovdexLibrary: INVALID_PAIR");
        if(sym_in == SOV.get_symbol()){   //SOV->XXX
            sovdex::sovtable _sovtbl( sovdex::code, sym_out.code().raw() );
            auto it = _sovtbl.begin();
            check(it != _sovtbl.end(), "SovdexLibrary: INVALID_PAIR");

            return { it->outstandingbal.amount, 2*it->connectorbal.amount };
        }
        if(sym_out == SOV.get_symbol()){   //SOV->XXX
            sovdex::sovtable _sovtbl( sovdex::code, sym_in.code().raw() );
            auto it = _sovtbl.begin();
            check(it != _sovtbl.end(), "SovdexLibrary: INVALID_PAIR");

            return { 2*it->connectorbal.amount, it->outstandingbal.amount };
        }
        check(false, "SovdexLibrary: Getting reserves for non-SOV pair");
        return {0, 0}; //unreachable
    }

    /**
     * ## STATIC `get_amount_out`
     *
     * Get expected conversion return
     *
     * ### params
     *
     * - `{asset} in` - amount in
     * - `{symbol} out_sym` - return symbol
     *
     * ### returns
     *
     * - `{asset}` - expected return
     *```
     */
    static asset get_amount_out(asset in, symbol out_sym) {

        check(in.symbol != out_sym, "SovdexLibrary: INVALID_PAIR");
        double fee = get_fee();
        if(in.symbol!=SOV.get_symbol() && out_sym!=SOV.get_symbol()) {      //if XXX->YYY - convert XXX to SOV first
            auto [res_in, res_out] = get_reserves(in.symbol, SOV.get_symbol());
            in.amount = res_out * (static_cast<double>(in.amount) / (res_in + in.amount));
            in.amount *= (10000-fee)/10000;
            in.symbol = SOV.get_symbol();
        }

        //now guaranteed to be XXX->SOV or SOV->YYY
        auto [res_in, res_out] = get_reserves(in.symbol, out_sym);
        auto amount_out = res_out * (static_cast<double>(in.amount) / (res_in + in.amount));
        amount_out *= (10000-fee)/10000;
        return { static_cast<int64_t>(amount_out), out_sym };
    }
}