# Deploy the latest eosio.contracts-1.9

*Date: 21 May 2020*

The current [BIOS Boot Sequence](https://developers.eos.io/welcome/latest/tutorials/bios-boot-sequence) on the Developer Portal suggests the following steps to bootstrap an EOSIO-2.0 blockchain:

1. build eosio.contracts-1.8 using eosio.cdt-1.6

2. build eosio.contracts-1.9 using eosio.cdt-1.7

which are troublesome and unnecessary.

In what follows, we demonstrate how to **directly deploy the latest eosio.contracts-1.9, built with the latest eosio.cdt-1.7, on an EOSIO-2.0 blockchain**.

This repository builds on the `releases/1.9.x` branch of `eosio.contracts`, with an `eosio.boot` contract added to it.

## Prerequisites

1. Make sure the EOSIO software is at the latest version, that is

    | Software          | Version |
    | ----------------- | ------- |
    | `eos`             | 2.0     |
    | `eosio.cdt`       | 1.7     |
    | `eosio.contracts` | 1.9     |

    and that commands like `nodeos`, `cleos`, `keosd` are directly accessible from the command line.

    *In particular, make sure the latest eosio.cdt-1.7 is properly installed.*

2. Make sure you have the default `keosd`-managed wallet and the wallet password to it. Make sure the development public key and private key are in it.

    | Type        | Key                                                     |
    | ----------- | ------------------------------------------------------- |
    | Public Key  | `EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV` |
    | Private Key | `5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3`   |

    *Wallet passwords differ on each machine. Mine is a 53-character string like `PW5...DN3`.*

3. Make sure you have the `curl` and `jq` tools on your machine. If not, try

    ```bash
    brew install curl jq
    ```

4. Clone the `1.9.x-with-boot` branch of this repository, using

    ```bash
    git clone https://github.com/b1aafulei/eosio.contracts --branch 1.9.x-with-boot con19 && cd con19
    ```

    Build the contracts in this repository, using

    ```bash
    ./build.sh -y
    ```

    *The above command and all the commands that follow should be run from the root of this repository folder.*


## Steps


1. Start `nodeos` to have a one-node blockchain, for example

```bash
nodeos \
--enable-stale-production \
--producer-name eosio \
--producer-name alice \
--plugin=eosio::chain_api_plugin \
--plugin=eosio::chain_plugin \
--plugin=eosio::history_plugin \
--plugin=eosio::http_plugin \
--plugin=eosio::net_plugin \
--plugin=eosio::producer_api_plugin \
--plugin=eosio::producer_plugin \
--delete-all-blocks >build/tmp.log 2>&1 &
```

To make sure `nodeos` is running properly, run

```bash
cleos get info
```

to check the status.

2. Unlock the wallet managed by `keosd`. Run

```bash
cleos wallet unlock
```

then enter the password. Alternatively, with your own `WALLET_PASSWORD` defined, run


```
echo $WALLET_PASSWORD | cleos wallet unlock
```

3. Activate the built-in protocol feature `PREACTIVATE_FEATURE` with

```bash
curl \
--request POST http://127.0.0.1:8888/v1/producer/schedule_protocol_feature_activations \
--data '{"protocol_features_to_activate":
        ["0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd"]}' \
--silent | jq
```

You should get

```
{
  "result": "ok"
}
```

4. Set `eosio.boot` contract on the `eosio` account

```bash
cleos set contract eosio ./build/contracts/eosio.boot
```

5. Activate the built-in protocol feature `WTMSIG_BLOCK_SIGNATURES` with the help from `eosio.boot`

```bash
cleos push action eosio activate \
'["299dcb6af692324b899b39f16d5a530a33062804e41f09dc97e9f156b4476707"]' \
--permission eosio@active
```

6. Get activated protocol features

```bash
curl \
--request POST http://127.0.0.1:8888/v1/chain/get_activated_protocol_features \
--data '{}' --silent | jq
```

You should get

```
{
  "activated_protocol_features": [
    {
      "feature_digest": "0ec7e080177b2c02b278d5088611686b49d739925a92d9bfcacd7fc6b74053bd",
      "activation_ordinal": 0,
      "activation_block_num": 172,
      "description_digest": "64fe7df32e9b86be2b296b3f81dfd527f84e82b98e363bc97e40bc7a83733310",
      "dependencies": [],
      "protocol_feature_type": "builtin",
      "specification": [
        {
          "name": "builtin_feature_codename",
          "value": "PREACTIVATE_FEATURE"
        }
      ]
    },
    {
      "feature_digest": "299dcb6af692324b899b39f16d5a530a33062804e41f09dc97e9f156b4476707",
      "activation_ordinal": 1,
      "activation_block_num": 1031,
      "description_digest": "ab76031cad7a457f4fd5f5fca97a3f03b8a635278e0416f77dcc91eb99a48e10",
      "dependencies": [],
      "protocol_feature_type": "builtin",
      "specification": [
        {
          "name": "builtin_feature_codename",
          "value": "WTMSIG_BLOCK_SIGNATURES"
        }
      ]
    }
  ]
}
```

7. Set `eosio.bios` contract on the `eosio` account

```bash
cleos set contract eosio ./build/contracts/eosio.bios
```

8. Test - Create test account `alice`

```bash
cleos create account eosio alice EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
```

9. Test - Set `alice` as a producer

```bash
cleos push action eosio setprods \
'{
    "schedule": [
        {
            "producer_name": "eosio",
            "authority": [
                "block_signing_authority_v0",
                {
                    "threshold": 1,
                    "keys": [
                        {
                            "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                            "weight": 1
                        }
                    ]
                }
            ]
        },
        {
            "producer_name": "alice",
            "authority": [
                "block_signing_authority_v0",
                {
                    "threshold": 1,
                    "keys": [
                        {
                            "key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV",
                            "weight": 1
                        }
                    ]
                }
            ]
        }
    ]
}' --permission eosio@active
```


10. Check test result

```bash
cleos get schedule
```

You should get

```
active schedule version 1
    Producer      Producer Authority
    ============= ==================
    eosio         {"threshold":1,"keys":[{"key":"EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight":1}]}
    alice         {"threshold":1,"keys":[{"key":"EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight":1}]}

pending schedule empty

proposed schedule empty
```

11. Stop `nodeos`

Finally, do not forget to stop `nodeos`, for example,

```bash
pkill nodeos
```
