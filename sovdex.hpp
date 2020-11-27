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
    typedef eosio::multi_index< "pair"_n, pair_row > pair;

    /**
     * ## STATIC `get_fee`
     *
     * Get sovdex total fee. 0.10% for each hop + 0.01% rounding error
     *
     * ### params
     *
     * - `{symbol} sym_in` - incoming symbol
     * - `{symbol} sym_out` - outgoing symbol
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
    static uint8_t get_fee(symbol& sym_in, symbol& sym_out)
    {
        return (sym_in == SOV.get_symbol() || sym_out == SOV.get_symbol()) ? 20 : 40;
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
    static std::pair<asset, asset> get_reserves( symbol sym_in, symbol sym_out)
    {
        check(sym_in!=sym_out, "SovdexLibrary: INVALID_PAIR");

        asset bal1, bal2, sovbal1, sovbal2, in_res, out_res;
        if(sym_in != SOV.get_symbol()){
            sovdex::pair _pair( code, sym_in.code().raw() );
            auto rowit = _pair.begin();
            check(rowit != _pair.end(), "SovdexLibrary: INVALID_SYMBOL");
            check(rowit->cw == 50, "SovdexLibrary: only 50/50 pools supported");
            sovbal1 = rowit->outstandingbal;
            bal1 = rowit->connectorbal;
        }
        if(sym_out != SOV.get_symbol()){
            sovdex::pair _pair( code, sym_out.code().raw() );
            auto rowit = _pair.begin();
            check(rowit != _pair.end(), "SovdexLibrary: INVALID_SYMBOL");
            check(rowit->cw == 50, "SovdexLibrary: only 50/50 pools supported");
            sovbal2 = rowit->outstandingbal;
            bal2 = rowit->connectorbal;
        }

        if(sym_in == SOV.get_symbol()){     //SOV->XXX
            in_res = sovbal1;
            out_res = bal1;
        }
        else if(sym_out == SOV.get_symbol()){   //XXX->SOV
            in_res = bal2;
            out_res = sovbal2;
        }
        else {      //XXX->YYY
            in_res = bal1;
            out_res.symbol = sym_out;
            out_res.amount = bal2.amount * ((double)sovbal1.amount / sovbal2.amount);
        }

        return {in_res, out_res};
    }
}