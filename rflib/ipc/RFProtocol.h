#ifndef __RFPROTOCOL_H__
#define __RFPROTOCOL_H__

#include <stdint.h>

#include "Ipc.h"
#include "IPAddress.h"
#include "MACAddress.h"
#include "converter.h"
#include "Action.hh"
#include "Match.hh"
#include "Option.hh"

enum {
	PORT_REGISTER,
	PORT_CONFIG,
	DATAPATH_PORT_REGISTER,
	DATAPATH_DOWN,
	VIRTUAL_PLANE_MAP,
	DATA_PLANE_MAP,
	ROUTE_MOD,
	CONTROLLER_REGISTER,
	ELECT_MASTER
};

class PortRegister : public IpcMessage {
    public:
        PortRegister();
        PortRegister(uint64_t vm_id, uint32_t vm_port, MACAddress hwaddress);

        uint64_t get_vm_id();
        void set_vm_id(uint64_t vm_id);

        uint32_t get_vm_port();
        void set_vm_port(uint32_t vm_port);

        MACAddress get_hwaddress();
        void set_hwaddress(MACAddress hwaddress);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t vm_id;
        uint32_t vm_port;
        MACAddress hwaddress;
};

class PortConfig : public IpcMessage {
    public:
        PortConfig();
        PortConfig(uint64_t vm_id, uint32_t vm_port, uint32_t operation_id);

        uint64_t get_vm_id();
        void set_vm_id(uint64_t vm_id);

        uint32_t get_vm_port();
        void set_vm_port(uint32_t vm_port);

        uint32_t get_operation_id();
        void set_operation_id(uint32_t operation_id);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t vm_id;
        uint32_t vm_port;
        uint32_t operation_id;
};

class DatapathPortRegister : public IpcMessage {
    public:
        DatapathPortRegister();
        DatapathPortRegister(uint64_t ct_id, uint64_t dp_id, uint32_t dp_port);

        uint64_t get_ct_id();
        void set_ct_id(uint64_t ct_id);

        uint64_t get_dp_id();
        void set_dp_id(uint64_t dp_id);

        uint32_t get_dp_port();
        void set_dp_port(uint32_t dp_port);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t ct_id;
        uint64_t dp_id;
        uint32_t dp_port;
};

class DatapathDown : public IpcMessage {
    public:
        DatapathDown();
        DatapathDown(uint64_t ct_id, uint64_t dp_id);

        uint64_t get_ct_id();
        void set_ct_id(uint64_t ct_id);

        uint64_t get_dp_id();
        void set_dp_id(uint64_t dp_id);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t ct_id;
        uint64_t dp_id;
};

class VirtualPlaneMap : public IpcMessage {
    public:
        VirtualPlaneMap();
        VirtualPlaneMap(uint64_t vm_id, uint32_t vm_port, uint64_t vs_id, uint32_t vs_port);

        uint64_t get_vm_id();
        void set_vm_id(uint64_t vm_id);

        uint32_t get_vm_port();
        void set_vm_port(uint32_t vm_port);

        uint64_t get_vs_id();
        void set_vs_id(uint64_t vs_id);

        uint32_t get_vs_port();
        void set_vs_port(uint32_t vs_port);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t vm_id;
        uint32_t vm_port;
        uint64_t vs_id;
        uint32_t vs_port;
};

class DataPlaneMap : public IpcMessage {
    public:
        DataPlaneMap();
        DataPlaneMap(uint64_t ct_id, uint64_t dp_id, uint32_t dp_port, uint64_t vs_id, uint32_t vs_port);

        uint64_t get_ct_id();
        void set_ct_id(uint64_t ct_id);

        uint64_t get_dp_id();
        void set_dp_id(uint64_t dp_id);

        uint32_t get_dp_port();
        void set_dp_port(uint32_t dp_port);

        uint64_t get_vs_id();
        void set_vs_id(uint64_t vs_id);

        uint32_t get_vs_port();
        void set_vs_port(uint32_t vs_port);

        virtual int get_type();


        virtual string str();

    private:
        uint64_t ct_id;
        uint64_t dp_id;
        uint32_t dp_port;
        uint64_t vs_id;
        uint32_t vs_port;
};

class RouteMod : public IpcMessage {
    public:
        RouteMod();
        RouteMod(uint8_t mod, uint64_t id, std::vector<Match> matches, std::vector<Action> actions, std::vector<Option> options);

        uint8_t get_mod();
        void set_mod(uint8_t mod);

        uint64_t get_id();
        void set_id(uint64_t id);

        std::vector<Match> get_matches();
        void set_matches(std::vector<Match> matches);
        void add_match(const Match& match);

        std::vector<Action> get_actions();
        void set_actions(std::vector<Action> actions);
        void add_action(const Action& action);

        std::vector<Option> get_options();
        void set_options(std::vector<Option> options);
        void add_option(const Option& option);

        virtual int get_type();


        virtual string str();

    private:
        uint8_t mod;
        uint64_t id;
        std::vector<Match> matches;
        std::vector<Action> actions;
        std::vector<Option> options;
};

class ControllerRegister : public IPCMessage {
    public:
        ControllerRegister();
        ControllerRegister(IPAddress ct_addr, uint32_t ct_port, string ct_role);

        IPAddress get_ct_addr();
        void set_ct_addr(IPAddress ct_addr);

        uint32_t get_ct_port();
        void set_ct_port(uint32_t ct_port);

        string get_ct_role();
        void set_ct_role(string ct_role);

        virtual int get_type();
        virtual string str();

    private:
        IPAddress ct_addr;
        uint32_t ct_port;
        string ct_role;
};

class ElectMaster : public IPCMessage {
    public:
        ElectMaster();
        ElectMaster(IPAddress ct_addr, uint32_t ct_port);

        IPAddress get_ct_addr();
        void set_ct_addr(IPAddress ct_addr);

        uint32_t get_ct_port();
        void set_ct_port(uint32_t ct_port);

        virtual int get_type();
        virtual string str();

    private:
        IPAddress ct_addr;
        uint32_t ct_port;
};

#endif /* __RFPROTOCOL_H__ */
