// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <common/signmessage.h>
#include <key.h>
#include <key_io.h>
#include <rpc/protocol.h>
#include <rpc/request.h>
#include <rpc/server.h>
#include <rpc/util.h>
#include <univalue.h>
#include <script/interpreter.h>
#include <rpc/rawtransaction_util.h>
#include <util/strencodings.h>
#include <primitives/transaction.h>
#include <core_io.h>
#include <logging.h>

#include <string>

static RPCHelpMan getdefaulttemplate()
{
    return RPCHelpMan{"getdefaulttemplate",
        "Calculate the BIP-119 default template for a raw transaction.",
        {
            {"txhex", RPCArg::Type::STR_HEX, RPCArg::Optional::NO, "The raw transaction."},
            {"input", RPCArg::Type::NUM, RPCArg::Optional::NO, "The integer index of the input to calculate the default template for."},
            {"iswitness", RPCArg::Type::BOOL, RPCArg::DefaultHint{"depends on heuristic tests"}, "Whether the transaction hex is a serialized witness transaction.\n"},
        },
        RPCResult{
            RPCResult::Type::STR_HEX, "", "The hex encoded BIP-119 default template calculated for this transaction"
        },
        RPCExamples{
            "\nCalculate the Default Template for a raw hexadecimal transaciton input 0\n"
            + HelpExampleCli("getdefaulttemplate", "\"hexstring\" 0 true")

            + HelpExampleRpc("getdefaulttemplate", "\"hexstring\", 0, true")
        },
        [&](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            std::string str_tx = self.Arg<std::string>("txhex");
            const uint32_t input_index{request.params[1].getInt<uint32_t>()};

            bool try_witness = request.params[2].isNull() ? true : request.params[2].get_bool();
            bool try_no_witness = request.params[2].isNull() ? true : !request.params[2].get_bool();

            CMutableTransaction mtx;

            if (!DecodeHexTx(mtx, str_tx, try_no_witness, try_witness)) {
                throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "TX decode failed");
            }

            const PrecomputedTransactionData data{mtx};

            uint256 ctv_hash = GetDefaultCheckTemplateVerifyHash(mtx, data.m_outputs_single_hash, data.m_sequences_single_hash, input_index);

            return HexStr(ctv_hash);
        },
    };
}

void RegisterCheckTemplateVerifyRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        {"util", &getdefaulttemplate},
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}
