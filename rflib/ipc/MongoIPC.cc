#include "MongoIPC.h"
#include <boost/thread.hpp>
#include "../defs.h"

MongoIPCMessageService::MongoIPCMessageService(const string &connectionUri, const string dbName, const string ipcUserId) {
    this->set_id(ipcUserId);
    this->dbName = dbName;
    this->connectionUri = connectionUri;
    this->databaseConnection = NULL;

    this->connect();
}

void MongoIPCMessageService::createChannel(const string &channelId) {
	this->connect();

	for (int i = 0; i < MONGO_MAX_RETRIES; i++)
	{
		string collectionNamespace = this->getNamespace(channelId);
		try {
			this->databaseConnection->createCollection(collectionNamespace, CC_SIZE, true);
			this->databaseConnection->ensureIndex(collectionNamespace, BSON("_id" << 1));
			this->databaseConnection->ensureIndex(collectionNamespace, BSON(TO_FIELD << 1));

			break;
		} catch (mongo::DBException &e) {

			if (i + 1 == MONGO_MAX_RETRIES) {
				stringstream error;
				error << "[ERROR]MongoIPC: Error creating database collection \n" <<
									 "       Caused By: " << e.what();

				throw new runtime_error(error.str());
			}

			cout << "[FAILOVER]MongoIPC: Error creating database collection. Trying again in " << MONGO_RETRY_INTERVAL <<
					" seconds. [" << (i + 1) <<	"/" << MONGO_MAX_RETRIES << "]" << endl;
			sleep(MONGO_RETRY_INTERVAL);
		}
	}
}


void MongoIPCMessageService::connect() {
	try {
		string error = "";
		mongo::ConnectionString connectionString = mongo::ConnectionString::parse(this->connectionUri, error);

		if(!connectionString.isValid())
			throw new std::runtime_error("[ERROR]MongoIPC: The Mongo Connection URI is invalid. Correct it and run me again.");

	    this->databaseConnection = connectionString.connect(error);

	    if(error != "")
	    	cout << "[WARNING]MongoIPC: A connection to the database could not be established" << endl <<
	    			"Caused By: " << error << endl;
	}
	catch(mongo::DBException &e) {
		cout << "[WARNING]MongoIPC: A connection to the database could not be established" << endl <<
				"Caused By: " << e.what() << endl;
	}
}

string MongoIPCMessageService::getNamespace(const string &collection) {
	return this->dbName + "." + collection;
}


void MongoIPCMessageService::listenWorker(const string &channelId, IPCMessageFactory *factory, IPCMessageProcessor *processor) {
	this->createChannel(channelId);

	mongo::Query query = QUERY(TO_FIELD << this->get_id() << READ_FIELD << false).sort("$natural");

	this->connect();

	//TODO auto_ptr is deprecated
	auto_ptr<mongo::DBClientCursor> cur;

	while (true) {

		for (int i = 0; i < MONGO_MAX_RETRIES; i++) {
			try {
				cur = this->databaseConnection->query(this->getNamespace(channelId), query, PENDINGLIMIT);

				break;
			} catch (mongo::DBException &e) {
				if (i + 1 == MONGO_MAX_RETRIES) {
					stringstream error;
					error << "[ERROR]MongoIPC: Error retrieving unread messages \n" <<
								   "Caused By: " << e.what();

					throw new runtime_error(error.str());
				}

				cout << "[FAILOVER]MongoIPC: Error retrieving unread messages. Trying again in " << MONGO_RETRY_INTERVAL <<
						" seconds. [" << (i + 1) << "/" << MONGO_MAX_RETRIES << "]" << endl;
				sleep(MONGO_RETRY_INTERVAL);
			}
		}

		if(cur.get() == NULL)
		{
			// no results, just continue
			continue;
		}


		mongo::BSONObj envelope;

		while (true) {
			if (!cur->more()) {
				if (cur->isDead()) {
					break;
				}
				sleep(1);
				continue;
			}

			try {
				envelope = cur->nextSafe();

			} catch (mongo::DBException &e) {
				break;

			}

			IPCMessage *msg = takeFromEnvelope(envelope, factory);
			processor->process(envelope["from"].String(), this->get_id(),
					channelId, *msg);
			delete msg;

			for (int i = 0; i < MONGO_MAX_RETRIES; i++) {
				try {
					this->databaseConnection->update(this->getNamespace(channelId), QUERY("_id" << envelope["_id"]),BSON("$set" << BSON(READ_FIELD << true)),false, true);

							break;
				} catch (mongo::DBException &e) {
					if(i + 1 == MONGO_MAX_RETRIES)
					{
						cout << "[ERROR]MongoIPC: The message (id: " << envelope["_id"] << ") could not be marked as read. Error: (" << e.what() << ")" << endl;
						exit(1);
					}

					cout << "[ERROR]MongoIPC: The message (id: " << envelope["_id"] << ") could not be marked as read. Trying again in " << MONGO_RETRY_INTERVAL << " seconds. [" << (i+1) << "]" << endl;
					sleep(MONGO_RETRY_INTERVAL);

				}
			}

			cout << "[OK]MongoIPC: The message (id: " << envelope["_id"]
					<< ") was marked as read."  << endl;

		}

		usleep(50000); // 50ms
	}
}

void MongoIPCMessageService::listen(const string &channelId, IPCMessageFactory *factory, IPCMessageProcessor *processor, bool block) {
    boost::thread t(&MongoIPCMessageService::listenWorker, this, channelId, factory, processor);
    if (block)
        t.join();
    else
        t.detach();
}

bool MongoIPCMessageService::send(const string &channelId, const string &to,
		IPCMessage& msg) {
	boost::lock_guard<boost::mutex> lock(ipcMutex);

cout << "send;" << endl;
	this->createChannel(channelId);
	this->connect();

	for (int i = 0; i < MONGO_MAX_RETRIES; i++) {

		try {
			this->databaseConnection->insert(this->getNamespace(channelId),
					putInEnvelope(this->get_id(), to, msg));
			break;

		} catch (mongo::DBException &e) {
			if (i + 1 == MONGO_MAX_RETRIES)

			{
				cout << "[ERROR]MongoIPC: Message could not be sent. Error: ("
						<< e.what() << ")" << endl;
				exit(1);
			}

			cout << "[ERROR]MongoIPC: Message not sent. Trying again in " << MONGO_RETRY_INTERVAL << " seconds. [" << (i
					+ 1) << "]" << endl;
			sleep(MONGO_RETRY_INTERVAL);
		}
	}

	cout << "[OK]MongoIPC: message sent" << endl;
	return true;
}


mongo::BSONObj putInEnvelope(const string &from, const string &to, IPCMessage &msg) {
    mongo::BSONObjBuilder envelope;

    envelope.genOID();
    envelope.append(FROM_FIELD, from);
    envelope.append(TO_FIELD, to);
    envelope.append(TYPE_FIELD, msg.get_type());
    envelope.append(READ_FIELD, false);

    const char* data = msg.to_BSON();
    envelope.append(CONTENT_FIELD, mongo::BSONObj(data));
    delete data;

    return envelope.obj();
}

IPCMessage* takeFromEnvelope(mongo::BSONObj envelope, IPCMessageFactory *factory) {
   IPCMessage* msg = factory->buildForType(envelope[TYPE_FIELD].Int());
   msg->from_BSON(envelope[CONTENT_FIELD].Obj().objdata());
   return msg;
}
