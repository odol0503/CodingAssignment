# CodingAssignment for StreamLabs

### Build

```
mkdir build
cd build
cmake -G [platform] .. (ex: cmake -G "Visual Studio 15 2017" ..)
```

### Binaries
```
bin/test_server.exe
bin/test_client.exe
(test_server.exe must be executed at the first.)
```

### Build With 
[JSON for Modern C++](https://github.com/nlohmann/json) - To serialize/deserialize

### How to test
Excute test_server.exe first, then test_client.ext.

There are automatical 8 tests.
1. The client sends integer value to the server, then receives the value that sent to the server synchronously.
2. The client sends string value to the server, then receives the value that sent to the server synchronously.
3. The client sends a certain structure to the server, then receives the structure that sent to the server synchronously.
4. The client calls the sum function on the server, then receives the result of the sum function synchronously.
5. The client sends integer value to the server, then receives the value that sent to the server asynchronously.
6. The client sends string value to the server, then receives the value that sent to the server asynchronously.
7. The client sends a certain structure to the server, then receives the structure that sent to the server asynchronously.
8. The client calls the reverse function on the server, then receives the result of the reverse function asynchronously.

### Implemenation
**common.hpp** 
- There are common definitions and classes for convenience.

**server.hpp** 
1. The server deserializes and classifies the serialized data that come from the client.
2. The server resends the data that come from the client to verify or calls the custom function that was registered on the server.

**client.hpp**
1. The client sends the serialized data to the server.
2. the client receives the data that come from the server, then verifies the retured data.
