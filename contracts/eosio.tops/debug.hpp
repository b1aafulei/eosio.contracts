#include <eosio/eosio.hpp>

namespace debug {
    struct foo {
        account_name from;
        account_name to;
        uint64_t amount;
        void print() const {
            eosio::print("Foo from ", eosio::name(from), " to ", eosio::name(to), " with amount ", amount, "\n");
        }
    };
}
