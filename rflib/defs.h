#ifndef __DEFS_H__
#define __DEFS_H__

/*
 * MONGO CONNECTION URI
 *
 * Single server connection: "host:port"
 * Replica Set connection:       "replica_set_name/host1:port1,host2:port2,...,hostN:portN"
 *
 */
#define MONGO_CONNECTION_URI "rs0/192.168.10.1:27017,192.168.10.1:27018,192.168.10.1:27019"
//#define MONGO_ADDRESS "rs0/192.169.1.1:27017,192.169.1.1:27018,192.169.1.1:27019"
#define MONGO_DB_NAME "db"
// When mongo connection fails, how often it will retry operation
#define MONGO_RETRY_INTERVAL 15 //seconds
// Maximum times to retry operation, when mongo connection fails
#define MONGO_MAX_RETRIES 3

#define RFCLIENT_RFSERVER_CHANNEL "rfclient<->rfserver"
#define RFSERVER_RFPROXY_CHANNEL "rfserver<->rfproxy"

#define RFSERVER_ID "rfserver"
#define RFPROXY_ID "rfproxy"

#define DEFAULT_RFCLIENT_INTERFACE "eth0"

#define SYSLOGFACILITY LOG_LOCAL7

#define RFVS_PREFIX 0x72667673
#define IS_RFVS(dp_id) !((dp_id >> 32) ^ RFVS_PREFIX)

#define RF_ETH_PROTO 0x0A0A /* RF ethernet protocol */

#define VLAN_HEADER_LEN 4
#define ETH_HEADER_LEN 14
#define ETH_CRC_LEN 4
#define ETH_PAYLOAD_MAX 1500
#define ETH_TOTAL_MAX (ETH_HEADER_LEN + ETH_PAYLOAD_MAX + ETH_CRC_LEN)
#define RF_MAX_PACKET_SIZE (VLAN_HEADER_LEN + ETH_TOTAL_MAX)

// We must match_l2 in order for packets to go up
#define MATCH_L2 true

typedef enum route_mod_type {
	RMT_ADD,			/* Add flow to datapath */
	RMT_DELETE			/* Remove flow from datapath */
	/* Future implementation */
	//RMT_MODIFY		/* Modify existing flow (Unimplemented) */
} RouteModType;

#define PC_MAP 0
#define PC_RESET 1

#define PRIORITY_BAND 0xA
#define PRIORITY_LOWEST 0x0000
#define PRIORITY_LOW 0x4010
#define PRIORITY_HIGH 0x8020
#define PRIORITY_HIGHEST 0xC030

#endif /* __DEFS_H__ */
