/*
    Copyright 2020 The Silkrpc Authors

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "chain.hpp"

#include <iterator>
#include <iostream>
#include <iomanip>
#include <utility>

#include <boost/endian/conversion.hpp>

#include <silkworm/common/util.hpp>
#include <silkworm/db/access_layer.hpp>
#include <silkworm/db/util.hpp>
#include <silkworm/execution/address.hpp>
#include <silkworm/rlp/decode.hpp>
#include <silkworm/rlp/encode.hpp>

#include <silkrpc/common/log.hpp>
#include <silkrpc/common/util.hpp>
#include <silkrpc/core/blocks.hpp>
#include <silkrpc/ethdb/cbor.hpp>
#include <silkrpc/ethdb/tables.hpp>
#include <silkrpc/json/types.hpp>
#include <silkrpc/types/log.hpp>
#include <silkrpc/types/receipt.hpp>

namespace silkrpc::core::rawdb {

asio::awaitable<uint64_t> read_header_number(const DatabaseReader& reader, const evmc::bytes32& block_hash) {
    const silkworm::ByteView block_hash_bytes{block_hash.bytes, silkworm::kHashLength};
    const auto kv_pair{co_await reader.get(silkrpc::db::table::kHeaderNumbers, block_hash_bytes)};
    const auto value = kv_pair.value;
    if (value.empty()) {
        throw std::invalid_argument{"empty block number value in read_header_number"};
    }
    co_return boost::endian::load_big_u64(value.data());
}

asio::awaitable<ChainConfig> read_chain_config(const DatabaseReader& reader) {
    const auto genesis_block_hash{co_await read_canonical_block_hash(reader, kEarliestBlockNumber)};
    SILKRPC_DEBUG << "rawdb::read_chain_config genesis_block_hash: " << genesis_block_hash << "\n";
    const silkworm::ByteView genesis_block_hash_bytes{genesis_block_hash.bytes, silkworm::kHashLength};
    const auto kv_pair{co_await reader.get(silkrpc::db::table::kConfig, genesis_block_hash_bytes)};
    const auto data = kv_pair.value;
    if (data.empty()) {
        throw std::invalid_argument{"empty chain config data in read_chain_config"};
    }
    SILKRPC_DEBUG << "rawdb::read_chain_config chain config data: " << data.c_str() << "\n";
    const auto json_config = nlohmann::json::parse(data.c_str());
    SILKRPC_TRACE << "rawdb::read_chain_config chain config JSON: " << json_config.dump() << "\n";
    co_return ChainConfig{genesis_block_hash, json_config};
}

asio::awaitable<uint64_t> read_chain_id(const DatabaseReader& reader) {
    const auto chain_info = co_await read_chain_config(reader);
    co_return chain_info.config["chainId"].get<uint64_t>();
}

asio::awaitable<evmc::bytes32> read_canonical_block_hash(const DatabaseReader& reader, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number);
    SILKRPC_TRACE << "rawdb::read_canonical_block_hash block_key: " << silkworm::to_hex(block_key) << "\n";
    const auto value{co_await reader.get_one(silkrpc::db::table::kCanonicalHashes, block_key)};
    if (value.empty()) {
        throw std::invalid_argument{"empty block hash value in read_canonical_block_hash"};
    }
    const auto canonical_block_hash{silkworm::to_bytes32(value)};
    SILKRPC_DEBUG << "rawdb::read_canonical_block_hash canonical block hash: " << canonical_block_hash << "\n";
    co_return canonical_block_hash;
}

asio::awaitable<intx::uint256> read_total_difficulty(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number, block_hash.bytes);
    const auto kv_pair{co_await reader.get(silkrpc::db::table::kDifficulty, block_key)};
    silkworm::ByteView value{kv_pair.value};
    if (value.empty()) {
        throw std::invalid_argument{"empty total difficulty value in read_total_difficulty"};
    }
    intx::uint256 total_difficulty{0};
    auto decoding_result{silkworm::rlp::decode(value, total_difficulty)};
    if (decoding_result != silkworm::rlp::DecodingResult::kOk) {
        throw std::runtime_error{"cannot RLP-decode total difficulty value in read_total_difficulty"};
    }
    co_return total_difficulty;
}

asio::awaitable<silkworm::BlockWithHash> read_block_by_hash(const DatabaseReader& reader, const evmc::bytes32& block_hash) {
    const auto block_number = co_await read_header_number(reader, block_hash);
    co_return co_await read_block(reader, block_hash, block_number);
}

asio::awaitable<silkworm::BlockWithHash> read_block_by_number(const DatabaseReader& reader, uint64_t block_number) {
    const auto block_hash = co_await read_canonical_block_hash(reader, block_number);
    co_return co_await read_block(reader, block_hash, block_number);
}

asio::awaitable<silkworm::BlockWithHash> read_block_by_transaction_hash(const DatabaseReader& reader, const evmc::bytes32& transaction_hash) {
    const silkworm::ByteView tx_hash{transaction_hash.bytes, silkworm::kHashLength};

    auto bytes = co_await reader.get_one(silkrpc::db::table::kTxLookup, tx_hash);
    if (bytes.empty()) {
        throw std::invalid_argument{"empty block number value in read_block_by_transaction_hash"};
    }
    auto block_number = std::stoul(silkworm::to_hex(bytes), 0, 16);
    SILKRPC_TRACE << "Block number " << block_number << " for transaction hash " << transaction_hash << "\n";
    co_return co_await core::rawdb::read_block_by_number(reader, block_number);
}

asio::awaitable<silkworm::BlockWithHash> read_block(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    auto header = co_await read_header(reader, block_hash, block_number);
    SILKRPC_INFO << "header: number=" << header.number << "\n";
    auto body = co_await read_body(reader, block_hash, block_number);
    SILKRPC_INFO << "body: #txn=" << body.transactions.size() << " #ommers=" << body.ommers.size() << "\n";
    silkworm::BlockWithHash block{silkworm::Block{body.transactions, body.ommers, header}, block_hash};
    co_return block;
}

asio::awaitable<silkworm::BlockHeader> read_header_by_hash(const DatabaseReader& reader, const evmc::bytes32& block_hash) {
    const auto block_number = co_await read_header_number(reader, block_hash);
    co_return co_await read_header(reader, block_hash, block_number);
}

asio::awaitable<silkworm::BlockHeader> read_header_by_number(const DatabaseReader& reader, uint64_t block_number) {
    const auto block_hash = co_await read_canonical_block_hash(reader, block_number);
    co_return co_await read_header(reader, block_hash, block_number);
}

asio::awaitable<silkworm::BlockHeader> read_header(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    auto data = co_await read_header_rlp(reader, block_hash, block_number);
    if (data.empty()) {
        throw std::runtime_error{"empty block header RLP in read_header"};
    }
    SILKRPC_TRACE << "data: " << silkworm::to_hex(data) << "\n";
    silkworm::ByteView data_view{data};
    silkworm::BlockHeader header{};
    const auto error = silkworm::rlp::decode(data_view, header);
    if (error != silkworm::rlp::DecodingResult::kOk) {
        throw std::runtime_error{"invalid RLP decoding for block header"};
    }
    co_return header;
}

asio::awaitable<silkworm::BlockBody> read_body(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto data = co_await read_body_rlp(reader, block_hash, block_number);
    if (data.empty()) {
        throw std::runtime_error{"empty block body RLP in read_body"};
    }
    SILKRPC_TRACE << "RLP data for block body #" << block_number << ": " << silkworm::to_hex(data) << "\n";

    try {
        silkworm::ByteView data_view{data};
        auto stored_body{silkworm::db::detail::decode_stored_block_body(data_view)};
        SILKRPC_DEBUG << "base_txn_id: " << stored_body.base_txn_id << " txn_count: " << stored_body.txn_count << "\n";
        auto transactions = co_await read_transactions(reader, stored_body.base_txn_id, stored_body.txn_count);

        silkworm::BlockBody body{transactions, stored_body.ommers};
        co_return body;
    } catch (silkworm::rlp::DecodingResult error) {
        SILKRPC_ERROR << "RLP decoding error for block body #" << block_number << " [" << static_cast<int>(error) << "]\n";
        throw std::runtime_error{"RLP decoding error for block body [" + std::to_string(static_cast<int>(error)) + "]"};
    }
}

asio::awaitable<silkworm::Bytes> read_header_rlp(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number, block_hash.bytes);
    const auto kv_pair = co_await reader.get(silkrpc::db::table::kHeaders, block_key);
    const auto data = kv_pair.value;
    co_return data;
}

asio::awaitable<silkworm::Bytes> read_body_rlp(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number, block_hash.bytes);
    const auto kv_pair = co_await reader.get(silkrpc::db::table::kBlockBodies, block_key);
    const auto data = kv_pair.value;
    co_return data;
}

asio::awaitable<std::optional<silkrpc::Transaction>> read_transaction_by_hash(const DatabaseReader& reader, const evmc::bytes32& transaction_hash) {
    const silkworm::ByteView tx_hash{transaction_hash.bytes, silkworm::kHashLength};

    auto bytes = co_await reader.get_one(silkrpc::db::table::kTxLookup, tx_hash);
    if (bytes.empty()) {
        SILKRPC_DEBUG << "No block number found for transaction hash " << transaction_hash << "\n";
        co_return std::nullopt;
    }
    auto block_number = std::stoul(silkworm::to_hex(bytes), 0, 16);
    SILKRPC_TRACE << "block number " << block_number << " for transaction hash " << transaction_hash << "\n";
    const auto block_with_hash = co_await core::rawdb::read_block_by_number(reader, block_number);
    const auto transactions = block_with_hash.block.transactions;

    for (size_t idx{0}; idx < transactions.size(); idx++) {
        auto ethash_hash{hash_of_transaction(transactions[idx])};
        silkworm::ByteView hash_view{ethash_hash.bytes, silkworm::kHashLength};
        SILKRPC_TRACE << "tx " << idx << ") hash: " << silkworm::to_bytes32(hash_view) << "\n";
        if (tx_hash == hash_view) {
            const auto block_header = block_with_hash.block.header;
            co_return silkrpc::Transaction{transactions[idx], block_with_hash.hash, block_header.number, block_header.base_fee_per_gas, idx};
        }
    }
    co_return std::nullopt;
}

asio::awaitable<Addresses> read_senders(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number, block_hash.bytes);
    const auto kv_pair = co_await reader.get(silkrpc::db::table::kSenders, block_key);
    const auto data = kv_pair.value;
    Addresses senders{data.size() / silkworm::kAddressLength};
    for (size_t i{0}; i < senders.size(); i++) {
        senders[i] = silkworm::to_address(silkworm::ByteView{&data[i * silkworm::kAddressLength], silkworm::kAddressLength});
    }
    co_return senders;
}

asio::awaitable<Receipts> read_raw_receipts(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    const auto block_key = silkworm::db::block_key(block_number);
    const auto kv_pair = co_await reader.get(silkrpc::db::table::kBlockReceipts, block_key);
    const auto data = kv_pair.value;
    SILKRPC_TRACE << "data: " << silkworm::to_hex(data) << "\n";
    if (data.empty()) {
        co_return Receipts{}; // TODO(canepat): use std::null_opt with asio::awaitable<std::optional<Receipts>>?
    }
    Receipts receipts{};
    const bool decoding_ok{cbor_decode(data, receipts)};
    if (!decoding_ok) {
        co_return receipts;
    }
    SILKRPC_DEBUG << "#receipts: " << receipts.size() << "\n";

    auto log_key = silkworm::db::log_key(block_number, 0);
    SILKRPC_DEBUG << "log_key: " << silkworm::to_hex(log_key) << "\n";
    Walker walker = [&](const silkworm::Bytes& k, const silkworm::Bytes& v) {
        auto tx_id = boost::endian::load_big_u32(&k[sizeof(uint64_t)]);
        const bool decoding_ok{cbor_decode(v, receipts[tx_id].logs)};
        if (!decoding_ok) {
            return false;
        }
        receipts[tx_id].bloom = bloom_from_logs(receipts[tx_id].logs);
        SILKRPC_DEBUG << "#receipts[" << tx_id << "].logs: " << receipts[tx_id].logs.size() << "\n";
        return true;
    };
    co_await reader.walk(silkrpc::db::table::kLogs, log_key, 8 * CHAR_BIT, walker);

    co_return receipts;
}

asio::awaitable<Receipts> read_receipts(const DatabaseReader& reader, const evmc::bytes32& block_hash, uint64_t block_number) {
    auto receipts = co_await read_raw_receipts(reader, block_hash, block_number);
    auto body = co_await read_body(reader, block_hash, block_number);
    auto senders = co_await read_senders(reader, block_hash, block_number);

    // Add derived fields to the receipts
    auto transactions = body.transactions;
    if (body.transactions.size() != receipts.size()) {
        throw std::runtime_error{"invalid receipt count in read_receipts"};
    }
    if (senders.size() != receipts.size()) {
        throw std::runtime_error{"invalid sender count in read_receipts"};
    }
    size_t log_index{0};
    for (size_t i{0}; i < receipts.size(); i++) {
        // The tx hash can be calculated by the tx content itself
        auto tx_hash{hash_of_transaction(transactions[i])};
        receipts[i].tx_hash = silkworm::to_bytes32(silkworm::full_view(tx_hash.bytes));
        receipts[i].tx_index = uint32_t(i);

        receipts[i].block_hash = block_hash;
        receipts[i].block_number = block_number;

        // When tx receiver is not set, create a contract with address depending on tx sender and its nonce
        if (!transactions[i].to.has_value()) {
            receipts[i].contract_address = silkworm::create_address(senders[i], transactions[i].nonce);
        }

        // The gas used can be calculated by the previous receipt
        if (i == 0) {
            receipts[i].gas_used = receipts[i].cumulative_gas_used;
        } else {
            receipts[i].gas_used = receipts[i].cumulative_gas_used - receipts[i-1].cumulative_gas_used;
        }

        receipts[i].from = senders[i];
        receipts[i].to = transactions[i].to;
        receipts[i].type = transactions[i].type;

        // The derived fields of receipt are taken from block and transaction
        for (size_t j{0}; j < receipts[i].logs.size(); j++) {
            receipts[i].logs[j].block_number = block_number;
            receipts[i].logs[j].block_hash = block_hash;
            receipts[i].logs[j].tx_hash = receipts[i].tx_hash;
            receipts[i].logs[j].tx_index = uint32_t(i);
            receipts[i].logs[j].index = log_index++;
            receipts[i].logs[j].removed = false;
        }
    }

    co_return receipts;
}

asio::awaitable<Transactions> read_transactions(const DatabaseReader& reader, uint64_t base_txn_id, uint64_t txn_count) {
    Transactions txns{};
    if (txn_count == 0) {
        SILKRPC_DEBUG << "txn_count: 0 #txns: 0\n";
        co_return txns;
    }

    txns.reserve(txn_count);

    silkworm::Bytes txn_id_key(8, '\0');
    boost::endian::store_big_u64(&txn_id_key[0], base_txn_id); // tx_id_key.data()?
    SILKRPC_DEBUG << "txn_id_key: " << silkworm::to_hex(txn_id_key) << "\n";
    size_t i{0};
    Walker walker = [&](const silkworm::Bytes&, const silkworm::Bytes& v) {
        SILKRPC_TRACE << "v: " << silkworm::to_hex(v) << "\n";
        silkworm::ByteView value{v};
        silkworm::Transaction tx{};
        const auto error = silkworm::rlp::decode(value, tx);
        if (error != silkworm::rlp::DecodingResult::kOk) {
            SILKRPC_ERROR << "invalid RLP decoding for transaction index " << i << "\n";
            return false;
        }
        SILKRPC_TRACE << "index: " << i << " tx_hash: " << silkworm::to_hex(hash_of(v).bytes) << "\n";
        txns.emplace(txns.end(), std::move(tx));
        i++;
        return i < txn_count;
    };
    co_await reader.walk(silkrpc::db::table::kEthTx, txn_id_key, 0, walker);

    SILKRPC_DEBUG << "#txns: " << txns.size() << "\n";

    co_return txns;
}

} // namespace silkrpc::core::rawdb
