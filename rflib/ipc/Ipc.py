class IpcMessage:
    """This is the base class for all IPC messages"""
    def __init__(self):
        self.__message_id = None
        self.__to = None
        self.__from = None
        self.__read = False

    def set_message_id(self, message_id):
        self.__message_id = message_id

    def get_message_id(self):
        return self.__message_id

    def set_to(self, to):
        self.__to = to

    def get_to(self):
        return self.__to

    def set_from(self, _from):
        self.__from = _from

    def get_from(self):
        return self.__from

    def set_read(self, read):
        self.__read = read

    def is_read(self):
        return self.__read

    def get_type(self):
        raise NotImplementedError
    
    def str(self):
        raise NotImplementedError

    def __str__(self):
        return self.str()

        
class IpcMessageProcessor:
    def process(self, ipc_message):
        raise NotImplementedError

        
class Ipc:
    def get_user_id(self):
        return self._user_id
    
    def set_user_id(self, user_id):
        self._user_id = user_id

    def get_channel_id(self):
        return self._channel_id
    
    def set_channel_id(self, channel_id):
        self._channel_id = channel_id
        
    def listen(self, message_processor):
        raise NotImplementedError

    def parallel_listen(self, message_processor):
        raise NotImplementedError
        
    def send(self, ipc_message):
        raise NotImplementedError


