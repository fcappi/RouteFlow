#ifndef IPC_H_
#define IPC_H_

#include <iostream>
#include <boost/function/function_fwd.hpp>
#include "IpcMessage.h"
#include "IpcMessageProcessor.h"


using namespace std;

class Ipc {
public:
	/**
	 * Send a message
	 *
	 * @param message The message
	 */
	virtual void send(IpcMessage* message) = 0;

	/**
	 * Start listening new messages and processing them, without use Thread
	 *
	 * @param messageProcessor object that will process the message
	 */
	virtual void listen(IpcMessageProcessor* messageProcessor) = 0;

	/**
	 * Start a thread to listening new messages and processing them
	 *
	 * @param messageProcessor object that will process the message
	 */
	virtual void parallelListen(IpcMessageProcessor* messageProcessor) = 0;

};

#endif /* IPC_H_ */
