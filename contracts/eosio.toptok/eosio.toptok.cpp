#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace eosiotoptok
{
class [[eosio::contract("eosio.toptok")]] ToptokContract : public eosio::contract
{
    struct TokenHolder
    {
        eosio::name  account;
        eosio::asset balance;

        uint64_t primary_key() const
        {
            return account.value;
        }

        uint64_t secondary_key() const
        {
            return balance.amount;
        }
    };

    using ToptokTable = eosio::multi_index<eosio::name("toptok"),
                                            TokenHolder,
                                            eosio::indexed_by<
                                                eosio::name("bybalance"),
                                                eosio::const_mem_fun<
                                                    TokenHolder,
                                                    uint64_t,
                                                    &TokenHolder::secondary_key>>>;

public:
    ToptokContract(eosio::name self,
           eosio::name first_receiver,
           eosio::datastream<const char *> ds)
                : eosio::contract(self, first_receiver, ds) {}

    [[eosio::on_notify("eosio.token::issue")]]
    void on_issue(const eosio::name & to, const eosio::asset & quantity)
    {
        eosio::print("eosio.token issued ", quantity, " to ", to, "\n");
        ToptokTable table(get_self(), get_self().value);
        ToptokTable::const_iterator it = table.find(to.value);
        if (it != table.end()) {
            table.modify(it, get_self(), [&](auto & row) {
                row.balance += quantity;
            });
        }
        else {
            table.emplace(get_self(), [&](auto & row) {
                row.account = to;
                row.balance = quantity;
            });
        }
    }

    [[eosio::on_notify("eosio.token::transfer")]]
    void on_transfer(eosio::name from, eosio::name to, eosio::asset value)
    {
        eosio::print(from, " transferred ", value, " to ", to, "\n");
        ToptokTable table(get_self(), get_self().value);
        ToptokTable::const_iterator it = table.find(to.value);
        if (it != table.end()) {
            table.modify(it, get_self(), [&](auto & row) {
                row.balance += value;
            });
        }
        else {
            table.emplace(get_self(), [&](auto & row) {
                row.account = to;
                row.balance = value;
            });
        }
        it = table.find(from.value);
        eosio::check(it != table.end(), "Cannot find " + from.to_string() + " in the database!");
        table.modify(it, get_self(), [&](auto & row) {
            row.balance -= value;
        });
    }

    [[eosio::action]]
    void get(eosio::name account)
    {
        ToptokTable table(get_self(), get_self().value);
        ToptokTable::const_iterator it = table.find(account.value);
        if (it != table.end())
            eosio::print("Account ", account, " has ", it-> balance, "\n");
        else
            eosio::print("Cannot find account ", account, "\n");
    }

    [[eosio::action]]
    void top(uint64_t n)
    {
        ToptokTable table(get_self(), get_self().value);
        auto index = table.get_index<eosio::name("bybalance")>();
        for (auto it = index.crbegin(); it != index.crend(); ++it) {
            eosio::print(it->account, " : ", it->balance, "\n");
            if (--n == 0)
                return;
        }
    }
};

}
