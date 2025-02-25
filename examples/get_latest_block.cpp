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

#include <silkrpc/config.hpp>

#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <utility>

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/detached.hpp>
#include <asio/thread_pool.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/use_future.hpp>
#include <grpcpp/grpcpp.h>
#include <silkworm/common/util.hpp>

#include <silkrpc/context_pool.hpp>
#include <silkrpc/common/constants.hpp>
#include <silkrpc/common/log.hpp>
#include <silkrpc/common/util.hpp>
#include <silkrpc/core/blocks.hpp>
#include <silkrpc/ethdb/transaction_database.hpp>
#include <silkrpc/ethdb/kv/remote_database.hpp>

ABSL_FLAG(std::string, target, silkrpc::kDefaultTarget, "server location as string <address>:<port>");
ABSL_FLAG(silkrpc::LogLevel, logLevel, silkrpc::LogLevel::Critical, "logging level");

using silkrpc::LogLevel;

asio::awaitable<std::optional<uint64_t>> latest_block(silkrpc::ethdb::Database& db) {
    std::optional<uint64_t> block_height;

    const auto db_transaction = co_await db.begin();
    try {
        silkrpc::ethdb::TransactionDatabase tx_db_reader{*db_transaction};
        block_height = co_await silkrpc::core::get_latest_block_number(tx_db_reader);
    } catch (const std::exception& e) {
        SILKRPC_ERROR << "exception: " << e.what() << "\n";
    } catch (...) {
        SILKRPC_ERROR << "unexpected exception\n";
    }
    co_await db_transaction->close();

    co_return block_height;
}

std::optional<uint64_t> get_latest_block(asio::io_context& io_context, silkrpc::ethdb::Database& db) {
    auto result = asio::co_spawn(io_context, latest_block(db), asio::use_future);
    return result.get();
}

int main(int argc, char* argv[]) {
    absl::SetProgramUsageMessage("Seek Turbo-Geth/Silkworm Key-Value (KV) remote interface to database");
    absl::ParseCommandLine(argc, argv);

    SILKRPC_LOG_VERBOSITY(absl::GetFlag(FLAGS_logLevel));

    try {
        auto target{absl::GetFlag(FLAGS_target)};
        if (target.empty() || target.find(":") == std::string::npos) {
            std::cerr << "Parameter target is invalid: [" << target << "]\n";
            std::cerr << "Use --target flag to specify the location of Turbo-Geth running instance\n";
            return -1;
        }

        // TODO(canepat): handle also secure channel for remote
        silkrpc::ChannelFactory create_channel = [&]() {
            return grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
        };
        // TODO(canepat): handle also local (shared-memory) database
        silkrpc::ContextPool context_pool{1, create_channel};
        auto& context = context_pool.get_context();
        auto& io_context = context.io_context;
        auto& database = context.database;
        auto context_pool_thread = std::thread([&]() { context_pool.run(); });

        const auto latest_block_number = get_latest_block(*io_context, *database);
        if (latest_block_number) {
            std::cout << "latest_block_number: " << latest_block_number.value() << "\n" << std::flush;
        }

        context_pool.stop();
        context_pool_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n" << std::flush;
    } catch (...) {
        std::cerr << "Unexpected exception\n" << std::flush;
    }

    return 0;
}
