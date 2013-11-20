#include <net/if.h>
#include <map>
#include <vector>

#include "ipc/Ipc.h"
#include "ipc/MongoIpc.h"
#include "ipc/RFProtocol.h"
#include "FlowTable.h"

class RFClient : private IpcMessageProcessor {
    public:
        RFClient(uint64_t id, const string &address);

    private:
        FlowTable* flowTable;
        Ipc* ipc;
        uint64_t id;

        map<string, Interface> ifacesMap;
        map<int, Interface> interfaces;
        vector<uint32_t> down_ports;

        uint8_t hwaddress[IFHWADDRLEN];
        int init_ports;

        void startFlowTable();
        void process(IpcMessage* msg);

        int send_packet(const char ethName[], uint64_t vm_id, uint8_t port);
        int set_hwaddr_byname(const char * ifname, uint8_t hwaddr[], int16_t flags);
        void load_interfaces();
        void send_port_map(uint32_t port);
};
