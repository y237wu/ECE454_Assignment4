server architecture:
- remote-access mode, unix semantics, centralized entry consistency
- mount/unmount registers and de-registers a client
- open/close directory lock and unlocks a read lock on directory
- open/close file lock and unlock a mutex on a file
- a client can only read/write once it obtain mutex on a file, otherwise it waits
- a client can only delete a file/folder once it obtain write lock on the folder/file
