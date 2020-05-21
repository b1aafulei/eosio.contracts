#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>

namespace eosiotops
{

class [[eosio::contract("eosio.tops")]] tops : public eosio::contract
{
private:

    struct [[eosio::table]] token_holders
    {
        eosio::asset  balance;
        eosio::name   holder;
        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };


    using top_token_holders = eosio::multi_index<eosio::name("top_token_holders"), token_holder>;



public:
    tops(eosio::name self, eosio::name first_receiver, eosio::datastream<const char *> ds) : eosio::contract(self, first_receiver, ds) {}


    [[eosio::on_notify("eosio.token::transfer")]]
    void listen(eosio::name from, eosio::name to, eosio::asset value, std::string memo)
    {
        eosio::print("from ", from, " to ", to, " at value ", value, " with memo ", memo, "\n");
        top_token_holders holders(get_first_receiver(), get_first_receiver().value);
        auto it = holders.find(from);
    }

    [[eosio::action]]
    void hello()
    {
        eosio::print("Hello, World! ~~~ from eosio.tops\n");
    }
};

}
