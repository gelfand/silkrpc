## RPC API Implementation Status

The following table shows the current [JSON RPC API](https://eth.wiki/json-rpc/API) implementation status in `Silkrpc`.

| Command                                 | Availability | Notes                                      |
| :-------------------------------------- | :----------: | -----------------------------------------: |
| web3_clientVersion                      | -            | not yet implemented                        |
| web3_sha3                               | -            | not yet implemented                        |
|                                         |              |                                            |
| net_listening                           | -            | not yet implemented                        |
| net_peerCount                           | -            | not yet implemented                        |
| net_version                             | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_blockNumber                         | Yes          |                                            |
| eth_chainID                             | -            | not yet implemented                        |
| eth_protocolVersion                     | -            | not yet implemented                        |
| eth_syncing                             | -            | not yet implemented                        |
| eth_gasPrice                            | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_getBlockByHash                      | Yes          | not yet tested for performance             |
| eth_getBlockByNumber                    | Yes          | not yet tested for performance             |
| eth_getBlockTransactionCountByHash      | -            | not yet implemented                        |
| eth_getBlockTransactionCountByNumber    | -            | not yet implemented                        |
| eth_getUncleByBlockHashAndIndex         | -            | not yet implemented                        |
| eth_getUncleByBlockNumberAndIndex       | -            | not yet implemented                        |
| eth_getUncleCountByBlockHash            | -            | not yet implemented                        |
| eth_getUncleCountByBlockNumber          | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_getTransactionByHash                | -            | not yet implemented                        |
| eth_getTransactionByBlockHashAndIndex   | -            | not yet implemented                        |
| eth_getTransactionByBlockNumberAndIndex | -            | not yet implemented                        |
| eth_getTransactionReceipt               | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_estimateGas                         | -            | not yet implemented                        |
| eth_getBalance                          | -            | not yet implemented                        |
| eth_getCode                             | -            | not yet implemented                        |
| eth_getTransactionCount                 | -            | not yet implemented                        |
| eth_getStorageAt                        | -            | not yet implemented                        |
| eth_call                                | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_newFilter                           | -            | not yet implemented                        |
| eth_newBlockFilter                      | -            | not yet implemented                        |
| eth_newPendingTransactionFilter         | -            | not yet implemented                        |
| eth_getFilterChanges                    | -            | not yet implemented                        |
| eth_uninstallFilter                     | -            | not yet implemented                        |
| eth_getLogs                             | Yes          |                                            |
|                                         |              |                                            |
| eth_accounts                            | No           | deprecated                                 |
| eth_sendRawTransaction                  | -            | not yet implemented, remote only           |
| eth_sendTransaction                     | -            | not yet implemented                        |
| eth_sign                                | No           | deprecated                                 |
| eth_signTransaction                     | -            | not yet implemented                        |
| eth_signTypedData                       | -            | ????                                       |
|                                         |              |                                            |
| eth_getProof                            | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_mining                              | -            | not yet implemented                        |
| eth_coinbase                            | -            | not yet implemented                        |
| eth_hashrate                            | -            | not yet implemented                        |
| eth_submitHashrate                      | -            | not yet implemented                        |
| eth_getWork                             | -            | not yet implemented                        |
| eth_submitWork                          | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_subscribe                           | -            | not yet implemented                        |
| eth_unsubscribe                         | -            | not yet implemented                        |
|                                         |              |                                            |
| debug_accountRange                      | -            | not yet implemented                        |
| debug_getModifiedAccountsByNumber       | -            | not yet implemented                        |
| debug_getModifiedAccountsByHash         | -            | not yet implemented                        |
| debug_storageRangeAt                    | -            | not yet implemented                        |
| debug_traceTransaction                  | -            | not yet implemented                        |
| debug_traceCall                         | -            | not yet implemented                        |
|                                         |              |                                            |
| trace_call                              | -            | not yet implemented                        |
| trace_callMany                          | -            | not yet implemented                        |
| trace_rawTransaction                    | -            | not yet implemented                        |
| trace_replayBlockTransactions           | -            | not yet implemented                        |
| trace_replayTransaction                 | -            | not yet implemented                        |
| trace_block                             | -            | not yet implemented                        |
| trace_filter                            | -            | not yet implemented                        |
| trace_get                               | -            | not yet implemented                        |
| trace_transaction                       | -            | not yet implemented                        |
|                                         |              |                                            |
| eth_getCompilers                        | No           | deprecated                                 |
| eth_compileLLL                          | No           | deprecated                                 |
| eth_compileSolidity                     | No           | deprecated                                 |
| eth_compileSerpent                      | No           | deprecated                                 |
|                                         |              |                                            |
| db_putString                            | No           | deprecated                                 |
| db_getString                            | No           | deprecated                                 |
| db_putHex                               | No           | deprecated                                 |
| db_getHex                               | No           | deprecated                                 |
|                                         |              |                                            |
| shh_post                                | No           | deprecated                                 |
| shh_version                             | No           | deprecated                                 |
| shh_newIdentity                         | No           | deprecated                                 |
| shh_hasIdentity                         | No           | deprecated                                 |
| shh_newGroup                            | No           | deprecated                                 |
| shh_addToGroup                          | No           | deprecated                                 |
| shh_newFilter                           | No           | deprecated                                 |
| shh_uninstallFilter                     | No           | deprecated                                 |
| shh_getFilterChanges                    | No           | deprecated                                 |
| shh_getMessages                         | No           | deprecated                                 |
|                                         |              |                                            |
| tg_getHeaderByHash                      | -            | not yet implemented                        |
| tg_getHeaderByNumber                    | -            | not yet implemented                        |
| tg_getLogsByHash                        | -            | not yet implemented                        |
| tg_forks                                | -            | not yet implemented                        |
| tg_issuance                             | -            | not yet implemented                        |

This table is constantly updated. Please visit again.
