cleos get account eosio.toptok &> /dev/null || cleos create account eosio eosio.toptok $(cat ~/public.key)

cleos set contract eosio.toptok .
