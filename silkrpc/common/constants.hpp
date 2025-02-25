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

#ifndef SILKRPC_COMMON_CONSTANTS_HPP_
#define SILKRPC_COMMON_CONSTANTS_HPP_

#include <chrono>

namespace silkrpc {

constexpr const char* kAddressPortSeparator{":"};

constexpr const char* kEmptyChainData{""};
constexpr const char* kDefaultLocal{"localhost:8545"};
constexpr const char* kDefaultTarget{"localhost:9090"};
constexpr const std::chrono::milliseconds kDefaultTimeout{10000};

} // namespace silkrpc

#endif  // SILKRPC_COMMON_CONSTANTS_HPP_
