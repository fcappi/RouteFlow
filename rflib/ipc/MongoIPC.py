import pymongo as mongo
import bson
import sys
import time
import rflib.ipc.IPC as IPC

from rflib.defs import *

FROM_FIELD = "from"
TO_FIELD = "to"
TYPE_FIELD = "type"
READ_FIELD = "read"
CONTENT_FIELD = "content"

# 1 MB for the capped collection
CC_SIZE = 1048576

def put_in_envelope(from_, to, msg):
    envelope = {}

    envelope[FROM_FIELD] = from_
    envelope[TO_FIELD] = to
    envelope[READ_FIELD] = False
    envelope[TYPE_FIELD] = msg.get_type()

    envelope[CONTENT_FIELD] = {}
    for (k, v) in msg.to_dict().items():
        envelope[CONTENT_FIELD][k] = v

    return envelope

def take_from_envelope(envelope, factory):
    msg = factory.build_for_type(envelope[TYPE_FIELD]);
    msg.from_dict(envelope[CONTENT_FIELD]);
    return msg;

def format_address(address):
    try:
        tmp = address.split(":")
        if len(tmp) == 2:
            return (tmp[0], int(tmp[1]))
        elif len(tmp) == 1:
            return (tmp[0],)
    except:
        raise ValueError, "Invalid address: " + str(address)
            
class MongoIPCMessageService(IPC.IPCMessageService):
    def __init__(self, address, db, id_, thread_constructor, sleep_function):
        """Construct an IPCMessageService

        Args:
            address: designates where the MongoDB instance is running.
            db: is the database name to connect to on MongoDB.
            id_: is an identifier to allow messages to be directed to the
                appropriate recipient.
            thread_constructor: function that takes 'target' and 'args'
                parameters for the function to run and arguments to pass, and
                return an object that has start() and join() functions.
            sleep_function: function that takes a float and delays processing
                for the specified period.
        """
        self._dbName = MONGO_DB_NAME
        self.address = format_address(address)
        self._id = id_
        self._producer_connection = None
        self._threading = thread_constructor
        self._sleep = sleep_function
        
        self.connect()
        
        self._create_channel(RFCLIENT_RFSERVER_CHANNEL)
        self._create_channel(RFSERVER_RFPROXY_CHANNEL)
        

    def connect(self):
        try:
            self._producer_connection = mongo.MongoClient(MONGO_ADDRESS, replicaSet='rs0')
            
            #self._producer_connection.write_concern = {'w':1, 'wtimeout': 1000}
            #print "DBINFO: Connection established with the following default ",
            #print "write concern -> ", self._producer_connection.write_concern
        except mongo.errors.AutoReconnect:
            print "Warning: Connection to Database failed. ",
            print "Trying to auto reconnect..."
        except mongo.errors.ConnectionFailure:
            print "Warning: Connection to Database failed. ",
            print "Some operations might not be completed!"
            print "ERR1; ", sys.exc_info()


    def listen(self, channel_id, factory, processor, block=True):
        worker = self._threading(target=self._listen_worker,
                                 args=(channel_id, factory, processor))
        worker.start()
        if block:
            worker.join()
        
    def send(self, channel_id, to, msg):
        #self._create_channel(channel_id)
        self.connect()
                    
        for i in xrange(0, MONGO_MAX_RETRIES):            
            try:                
                collection = self._producer_connection[self._dbName][channel_id]
                collection.insert(put_in_envelope(self.get_id(), to, msg))
                
                break;
                           
            except:
                if (i + 1) == MONGO_MAX_RETRIES:
                    print "[ERROR]MongoIPC: Message could not be sent. ",
                    print "Error: (", sys.exc_info(), ")"
                    sys.exit(1)
                        
                print "[RECOVERING]MongoIPC: Message not sent. ",
                print "Trying again in ", MONGO_RETRY_INTERVAL, " seconds. ",
                print "[", (i+1), "]"
                
                time.sleep(MONGO_RETRY_INTERVAL)
                
        print "[OK]MongoIPC: Message sent"
        return True


    def _listen_worker(self, channel_id, factory, processor):
        
        #self._create_channel(channel_id)
        self.connect()
        
        while True:            
            # tries to get unread messages
            for i in xrange(0, MONGO_MAX_RETRIES):            
                try:
                    collection = self._producer_connection[self._dbName][channel_id]
                    cursor = collection.find(
                        {TO_FIELD: self.get_id(), READ_FIELD: False},
                        tailable=True
                    )
                    
                    #cursor OK, break for
                    break
                
                except:                    
                    if (i + 1) == MONGO_MAX_RETRIES:
                        print "[ERROR]MongoIPC: Could not get unread messages. Error: (", sys.exc_info(), ")"                                   
                        sys.exit(2)
                        
                    print "[RECOVERING]MongoIPC: Could not get unread messages. Trying again in ", MONGO_RETRY_INTERVAL, " seconds. [",  (i+1),  "]"                
                    time.sleep(MONGO_RETRY_INTERVAL)
            
            while cursor.alive:
                
                try:
                    envelope = cursor.next()
                    
                except StopIteration:
                    time.sleep(1)
                    continue
                except:
                    #print "[RECOVERING]MongoIPC: Fail to reach messages. Err:",sys.exc_info()
                    break;
                
                msg = take_from_envelope(envelope, factory)
                processor.process(envelope[FROM_FIELD], envelope[TO_FIELD], channel_id, msg);
                        
                # tries to mark message as read
                for j in xrange(0, MONGO_MAX_RETRIES):                            
                    try:
                        collection = self._producer_connection[self._dbName][channel_id]
                        collection.update({"_id": envelope["_id"]},
                                          {"$set": {READ_FIELD: True}})                                
                                                       
                        # update done, break for
                        break
                        
                    except:                                
                        if (j + 1) == MONGO_MAX_RETRIES:
                            print "[ERROR]MongoIPC: The Message (id: ",
                            print envelope["_id"], 
                            print ") could not be marked as read. ",
                            print "Error: (", sys.exc_info, ")"
                            sys.exit(1)
                                                                                           
                        print "[RECOVERING]MongoIPC: Could not mark message ",
                        print "as read. Trying again in ",
                        print MONGO_RETRY_INTERVAL, " seconds. [", (j+1), "]"
                        time.sleep(MONGO_RETRY_INTERVAL)
                    
                print "[OK]MongoIPC: Message (id: ", envelope["_id"], ") was marked as Read."    
                    
            self._sleep(0.05)
                
    def _create_channel(self, name):
        self.connect()
                        
        for i in xrange(0, MONGO_MAX_RETRIES):            
            try:
                db = self._producer_connection[self._dbName]
                
                collection = mongo.collection.Collection(db, name, True,
                                                         capped=True, 
                                                         size=CC_SIZE)
                collection.ensure_index([("_id", mongo.ASCENDING)])
                collection.ensure_index([(TO_FIELD, mongo.ASCENDING)])
                
                
                break;
                        
            except:
                None              
                #if (i + 1) == MONGO_MAX_RETRIES:
                #print "[ERROR]MongoIPC: Channel ", name, " not created. Error: (", sys.exc_info(), ")"
                #    
                #    sys.exit(1)
                #
                #print "[RECOVERING]MongoIPC: Channel ", name, " not created. Trying again in ", MONGO_RETRY_INTERVAL, " seconds. [", (i+1), "]"
                #time.sleep(MONGO_RETRY_INTERVAL)
             
        print "[OK]MongoIPC: Channel ", name, "successful created"

class MongoIPCMessage(dict, IPC.IPCMessage):
    def __init__(self, type_, **kwargs):
        dict.__init__(self)
        self.from_dict(kwargs)
        self._type = type_
        
    def get_type(self):
        return self._type

    def from_dict(self, data):
        for (k, v) in data.items():
            self[k] = v
        
    def from_bson(self, data):
        data = bson.BSON.decode(data)
        self.from_dict(data)

    def to_bson(self):
        return bson.BSON.encode(self)
        
    def str(self):
        string = ""
        for (k, v) in self.items():
            string += str(k) + ": " + str(v) + "\n"
        return string
        
    def __str__(self):
        return IPC.IPCMessage.__str__(self)
