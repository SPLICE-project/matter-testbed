#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <string>
#include <sqlite3.h>
#include <vector>
#include "google/protobuf/util/time_util.h"

#include "splice_cube.grpc.pb.h"

#define DB_PATH "/home/initialdev/Documents/Projects/splice/splice/test/splicecube.db"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using splice_cube::SingleMACAddr;
using splice_cube::Time;
using splice_cube::MUDProfile;
using splice_cube::DoubleMACAddr;
using splice_cube::PacketData;
using splice_cube::CubeletInfo;
using splice_cube::Packets;
using splice_cube::SPLICEcubeServer;
using splice_cube::DeviceInfo;


// Server Implementation
class SPLICEcubeServerImpl final : public SPLICEcubeServer::Service {
public:
    sqlite3* db;
    int rc{};
    char *zErrMsg = nullptr;
    
    explicit SPLICEcubeServerImpl(sqlite3 *db){
        this->db = db;
    }

    Status getPacketsTo(ServerContext* context, const SingleMACAddr* request,
                        Packets* reply) override {

        std::vector<PacketData> packets;

        char *sql_query = sqlite3_mprintf("SELECT * FROM wifi_traffic WHERE dst_mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getPackets_Callback, &packets,
                     &this->zErrMsg);

        for (const auto & packet : packets){
            PacketData *pd = reply->add_packets();
            pd->set_id(packet.id());
            pd->set_time(packet.time());
            pd->set_src_mac(packet.src_mac());
            pd->set_dst_mac(packet.dst_mac());
            pd->set_src_ip(packet.src_ip());
            pd->set_dst_ip(packet.dst_ip());
            pd->set_rssi(packet.rssi());
            pd->set_notes(packet.notes());

        }
        return Status::OK;
    }

    Status getPacketsFrom(ServerContext* context, const SingleMACAddr* request,
                        Packets* reply) override {

        std::vector<PacketData> packets;

        char *sql_query = sqlite3_mprintf("SELECT * FROM wifi_traffic WHERE src_mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getPackets_Callback, &packets,
                     &this->zErrMsg);

        for (const auto & packet : packets){
            PacketData *pd = reply->add_packets();
            pd->set_id(packet.id());
            pd->set_time(packet.time());
            pd->set_src_mac(packet.src_mac());
            pd->set_dst_mac(packet.dst_mac());
            pd->set_src_ip(packet.src_ip());
            pd->set_dst_ip(packet.dst_ip());
            pd->set_rssi(packet.rssi());
            pd->set_notes(packet.notes());

        }
        return Status::OK;
    }

    Status getPacketsFromTo(ServerContext* context, const DoubleMACAddr* request,
                          Packets* reply) override {
        std::vector<PacketData> packets;

        char *sql_query = sqlite3_mprintf("SELECT * FROM wifi_traffic WHERE src_mac_addr = \"%s\" AND dst_mac_addr =  \"%s\";",
                                          request->src_mac().c_str(), request->dst_mac().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getPackets_Callback, &packets,
                     &this->zErrMsg);

        for (const auto & packet : packets){
            PacketData *pd = reply->add_packets();
            pd->set_id(packet.id());
            pd->set_time(packet.time());
            pd->set_src_mac(packet.src_mac());
            pd->set_dst_mac(packet.dst_mac());
            pd->set_src_ip(packet.src_ip());
            pd->set_dst_ip(packet.dst_ip());
            pd->set_rssi(packet.rssi());
            pd->set_notes(packet.notes());

        }
        return Status::OK;
    }

    Status getCubeletInfo(ServerContext* context, const SingleMACAddr* request,
                          CubeletInfo* reply) override {

        char *sql_query = sqlite3_mprintf("SELECT * FROM cubelets where mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getCubeletInfo_Callback, reply,
                     &this->zErrMsg);

        return Status::OK;
    }

    Status getCubeletLastSeen(ServerContext* context, const SingleMACAddr* request,
                              Time *reply) override {

        char *sql_query = sqlite3_mprintf("SELECT last_seen FROM cubelets where mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getLastSeen_Callback, reply,
                     &this->zErrMsg);

        return Status::OK;

    }

    Status getDeviceLastSeen(ServerContext* context, const SingleMACAddr* request,
                             Time* reply) override {

        char *sql_query = sqlite3_mprintf("SELECT last_seen FROM device_inventory where mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getLastSeen_Callback, reply,
                     &this->zErrMsg);

        return Status::OK;
    }

    Status getDeviceInfo(ServerContext* context, const SingleMACAddr* request,
                         DeviceInfo* reply) override {
        char *sql_query = sqlite3_mprintf("SELECT * FROM device_inventory where mac_addr = \"%s\";", request->mac_address().c_str());

        sqlite3_exec(this->db, sql_query, SPLICEcubeServerImpl::getDeviceInfo_Callback, reply,
                     &this->zErrMsg);

        return Status::OK;
    }



private:

    static int getPackets_Callback(void *ptr, int argc, char **argv, char **azColName){

        auto *packets = reinterpret_cast<std::vector<PacketData> *>(ptr);
        PacketData pd;

        pd.set_id(atoi(argv[0]));
        pd.set_time(argv[1]);
        pd.set_src_ip(argv[2]);
        pd.set_dst_ip(argv[3]);
        pd.set_src_mac(argv[4]);
        pd.set_dst_mac(argv[5]);
        pd.set_rssi(atoi(argv[6]));
        pd.set_notes(argv[7] ? argv[7] : "NULL");

        packets->push_back(pd);

    }
    static int getCubeletInfo_Callback(void *ptr, int argc, char **argv, char **azColName){
        auto *cubelet_info = reinterpret_cast<CubeletInfo*>(ptr);

        cubelet_info->set_cubelet_num(atoi(argv[0]));
        cubelet_info->set_ip_addr(argv[1]);
        cubelet_info->set_mac_addr(argv[2]);
        cubelet_info->set_hostname(argv[3]);
        cubelet_info->set_time_added(argv[4]);
        cubelet_info->set_last_seen(argv[5]);
        cubelet_info->set_notes(argv[6] ? argv[6] : "NULL");

    }

    static int getLastSeen_Callback(void *ptr, int argc, char **argv, char **azColName){
        auto *t = reinterpret_cast<Time*>(ptr);
        t->set_time(argv[0]);
    }

    static int getDeviceInfo_Callback(void *ptr, int argc, char **argv, char **azColName){
        auto *device = reinterpret_cast<DeviceInfo*>(ptr);

        device->set_id(atoi(argv[0]));
        device->set_name(argv[1]);
        device->set_vendor(argv[2]);
        device->set_mac_addr(argv[3]);
        device->set_ip_addr(argv[4]);
        device->set_time_added(argv[5]);
        device->set_last_seen(argv[6]);
        device->set_notes(argv[7] ? argv[7]: "NULL");

    }


};

void RunServer() {
    sqlite3 *db;

    int rc = sqlite3_open(DB_PATH, &db);

    if (rc){
        std::cout << "Error opening SPLICEcube database." << std::endl;
    }

    std::string server_address("0.0.0.0:50052");
    SPLICEcubeServerImpl service(db);

    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: " << server_address << std::endl;

    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}