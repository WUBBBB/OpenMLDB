/*
 * replicate_node.h
 * Copyright (C) 2017 4paradigm.com
 * Author denglong
 * Date 2017-08-11
 *
*/
#ifndef RTIDB_REPLICATE_NODE_H
#define RTIDB_REPLICATE_NODE_H

#include <vector>
#include "base/skiplist.h"
#include "boost/function.hpp"
#include "log/log_writer.h"
#include "log/log_reader.h"
#include "log/sequential_file.h"
#include "rpc/rpc_client.h"
#include "proto/tablet.pb.h"

namespace rtidb {
namespace replica {

using ::rtidb::log::SequentialFile;
using ::rtidb::log::Reader;

const static uint32_t FOLLOWER_REPLICATE_MODE = 0;
const static uint32_t SNAPSHOT_REPLICATE_MODE = 1;

typedef ::rtidb::base::Skiplist<uint32_t, uint64_t, ::rtidb::base::DefaultComparator> LogParts;

class ReplicateNode {
public:
    ReplicateNode(const std::string& point, LogParts* logs, const std::string& log_path, uint32_t tid, uint32_t pid);
    virtual ~ReplicateNode();
    ::rtidb::base::Status ReadNextRecord(::rtidb::base::Slice* record, std::string* buffer);
    int RollRLogFile();
    int OpenSeqFile(const std::string& path);
    uint32_t GetMode() {
        return replicate_node_mode_;
    }
    virtual int SyncData(uint64_t log_offset) = 0;
    virtual int MatchLogOffsetFromNode() = 0;
    void GoBackToLastBlock();
    bool IsLogMatched();
    void SetLogMatch(bool log_match);
    std::string GetEndPoint();
    uint64_t GetLastSyncOffset();
    int GetLogIndex();
    void SetLastSyncOffset(uint64_t offset);
    ReplicateNode(const ReplicateNode&) = delete;
    ReplicateNode& operator= (const ReplicateNode&) = delete;
protected:
    std::string endpoint;
    uint64_t last_sync_offset_;
    std::string log_path_;
    int log_part_index_;
    SequentialFile* sf_;
    Reader* reader_;
    LogParts* logs_;
    uint32_t replicate_node_mode_;
    bool log_matched_;
    uint32_t tid_;
    uint32_t pid_;
};

class FollowerReplicateNode: public ReplicateNode {
public:
    FollowerReplicateNode(const std::string& point, LogParts* logs, const std::string& log_path,
            uint32_t tid, uint32_t pid, ::rtidb::RpcClient* rpc_client);
    int MatchLogOffsetFromNode();        
    int SyncData(uint64_t log_offset);
    FollowerReplicateNode(const FollowerReplicateNode&) = delete;
    FollowerReplicateNode& operator= (const FollowerReplicateNode&) = delete;

private:
    std::vector<::rtidb::api::AppendEntriesRequest> cache_;
    ::rtidb::RpcClient* rpc_client_;
};

}
}

#endif
