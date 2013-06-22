#ifndef __MONGOIPC_H__
#define __MONGOIPC_H__

#include <mongo/client/dbclient.h>
#include <mongo/client/dbclient_rs.h>
#include "IPC.h"

#define FROM_FIELD "from"
#define TO_FIELD "to"
#define TYPE_FIELD "type"
#define READ_FIELD "read"
#define CONTENT_FIELD "content"

// 1 MB for the capped collection
#define CC_SIZE 1048576

// Handle a maximum of 10 messages at a time
#define PENDINGLIMIT 10

mongo::BSONObj putInEnvelope(const string &from, const string &to,
		IPCMessage &msg);
IPCMessage* takeFromEnvelope(mongo::BSONObj envelope,
		IPCMessageFactory *factory);

/** An IPC message service that uses MongoDB as its backend. */
class MongoIPCMessageService: public IPCMessageService {
public:
	/** Creates and starts an IPC message service using MongoDB.
	 @param connectionUri MongoDB connection URI used to connect to a MongoDB database server
	 @param dbName The name of the database to use
	 @param ipcUserId The ID of this IPC service user*/
	MongoIPCMessageService(const string &connectionUri, const string dbName,
			const string ipcUserId);
	virtual void listen(const string &channelId, IPCMessageFactory *factory,
			IPCMessageProcessor *processor, bool block = true);
	virtual bool send(const string &channelId, const string &to,
			IPCMessage& msg);

private:
	string dbName;
	string connectionUri;
	mongo::DBClientBase *databaseConnection;
	boost::mutex ipcMutex;
	void listenWorker(const string &channelId, IPCMessageFactory *factory,
			IPCMessageProcessor *processor);
	void createChannel(const string &ns);
	/*
	 * Establishes a connection with the database
	 */
	void connect();
	/**
	 * returns collection namespace in the format: database_name.collection
	 *
	 * @param collection The collection
	 * @return The namespace
	 */
	string getNamespace(const string &collection);
};

#endif /* __MONGOIPC_H__ */
