/*
   Copyright 2021 The Silkrpc Authors

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

#include <optional>
#include <string>
#include <vector>

#include "call.hpp"
#include <catch2/catch.hpp>
#include <evmc/evmc.hpp>
#include <intx/intx.hpp>
#include <silkworm/common/util.hpp>

namespace silkrpc {

using Catch::Matchers::Message;
using evmc::literals::operator""_address, evmc::literals::operator""_bytes32;
using silkworm::kGiga;

TEST_CASE("create empty call", "[silkrpc][types][call]") {
    Call call{};
    CHECK(call.from == std::nullopt);
    CHECK(call.to == std::nullopt);
    CHECK(call.gas == std::nullopt);
    CHECK(call.gas_price == std::nullopt);
    CHECK(call.max_priority_fee_per_gas == std::nullopt);
    CHECK(call.max_fee_per_gas == std::nullopt);
    CHECK(call.value == std::nullopt);
    CHECK(call.data == std::nullopt);
}

TEST_CASE("create call with gasprice", "[silkrpc][types][call]") {
    Call call{
       std::nullopt,
       std::nullopt,
       235,                 // gas
       21000,               // gas_price
       std::nullopt,
       std::nullopt,
       31337,               // value
       {},                  // data
    };
    silkworm::Transaction txn = call.to_transaction();
    CHECK(txn.gas_limit == 235);
    CHECK(txn.max_fee_per_gas == 21000);
    CHECK(txn.max_priority_fee_per_gas == 21000);
}

TEST_CASE("create call with no gasprice & not max_fee_per_gas and max_priority_fee_per_gas ", "[silkrpc][types][call]") {
    Call call{
       std::nullopt,
       std::nullopt,
       235,                 // gas
       0,                   // gas_price
       std::nullopt,
       std::nullopt,
       std::nullopt,
       std::nullopt,
    };
    silkworm::Transaction txn = call.to_transaction();
    CHECK(txn.gas_limit == 235);
    CHECK(txn.max_fee_per_gas == 0);
    CHECK(txn.max_priority_fee_per_gas == 0);
}

TEST_CASE("create call with no gasprice & valid max_fee_per_gas and max_priority_fee_per_gas ", "[silkrpc][types][call]") {
    Call call{
       0x99f9b87991262f6ba471f09758cde1c0fc1de734_address,  // from
       0x5df9b87991262f6ba471f09758cde1c0fc1de734_address,  // to
       235,                 // gas
       0,                   // gas_price
       10000,               // max_fee_per_gas
       10000,               // max_priority_fee_per_gas
       31337,               // value
       *silkworm::from_hex("001122aabbcc")
    };
    silkworm::Transaction txn = call.to_transaction();
    CHECK(txn.from == 0x99f9b87991262f6ba471f09758cde1c0fc1de734_address);
    CHECK(txn.to == 0x5df9b87991262f6ba471f09758cde1c0fc1de734_address);
    CHECK(txn.gas_limit == 235);
    CHECK(txn.max_fee_per_gas == 0);
    CHECK(txn.max_priority_fee_per_gas == 0);
    CHECK(txn.value == 31337);
    CHECK(txn.data == *silkworm::from_hex("001122aabbcc"));
}

TEST_CASE("create call with no gas", "[silkrpc][types][call]") {
    Call call;
    silkworm::Transaction txn = call.to_transaction();
    CHECK(txn.gas_limit == 30000000);
    CHECK(txn.value == 0);
    CHECK(txn.data == silkworm::Bytes{});
}

} // namespace silkrpc

