cleos get account eosio.token &> /dev/null || cleos create account eosio eosio.token $(cat ~/public.key)

cleos set contract eosio.token .
