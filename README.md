# BlockChainServer

The BlockChainServer application has been tested on windows and mac. It has not been tested on Linux, but it will probably work. The server uses 2 ports. The 8080 port is for the API interface and the 8081 is for communicating between servers.

### Compiling
On unix
```
mkdir build
cd build
cmake ..
make
make install
```
On windows
You can create VS files using cmake-gui.

### Runnig
You can use --http-port to change the default HTTP port and --tcp-port to change the TCP port. Example usage:
```
./blockchainserver --tcp-port 81 --http-port 80
```

### API
For getting help 
```
http://127.0.0.1:8080/help 
```

##### /createblock
It is using to create a new block.
```sh
curl -X POST -d "Test Data" http://127.0.0.1:8080/createblock
```
```json
{"Status":true,"Index":2}
```

#### /getblock
Get block information via index.
```sh
curl -X POST -d "2" http://127.0.0.1:8080/getblock
```
```json
{"Status":true,"Data":"VGVzdCBEYXRh"}
```
Data is encoded in Base64.

#### /totalblocks
Total blocks count.
```sh
curl http://127.0.0.1:8080/totalblocks
```
```json
{"Status":true,"TotalBlock":2}
```

#### /blocks
Fetch all blocks.
```sh
curl http://127.0.0.1:8080/blocks
```
```json
{"Status":true,"Blocks":[{"Index":1,"Hash":"d9b5d942a14db7a408328b178bfb531b508b322a08264cbf189c008947512634","PreviousHash":"8bb60170a7a13686c3c651dac9038ce96eed1ff208cd5e29b4b16bbfec5c9c20","TimeStamp":1508090809,"Nonce":0,"Data":"R2VuZXNpcyBibG9jaw=="},{"Index":2,"Hash":"35da1352558b78801f3ed2c91520ab2bffac7fceaf3338b2c767f2a463aab3ec","PreviousHash":"d9b5d942a14db7a408328b178bfb531b508b322a08264cbf189c008947512634","TimeStamp":1508236436,"Nonce":0,"Data":"VGVzdCBEYXRh"}]}
```

#### /addnode
Add new blockchain server.
```sh
curl -X POST -d "127.0.0.1:9787" http://127.0.0.1:8080/addnode
```
```json
{"Status":true,"Message":"Node will be added."}
```
Synchronization could take a time. 

#### /nodes
Connected node list.
```sh
curl http://127.0.0.1:8080/nodes
```
```json
{"Status":true,"Nodes":["127.0.0.1:9787"]}
```

#### /removenode
Disconnect from remote node.
```sh
curl -X POST -d "127.0.0.1:9787" http://127.0.0.1:8080/removenode
```
```json
{"Status":true}
```

#### /sync
The system automatically synchronize other servers continuously but can be used if you want to trigger this manually.
```sh
curl -X POST -d "127.0.0.1:9787" http://127.0.0.1:8080/removenode
```
```json
{"Status":true}
```

#### /validate
It is used to check the validity of block information.
```sh
curl -X POST -d "{\"Index\":2,\"Hash\":\"35da1352558b78801f3ed2c91520ab2bffac7fceaf3338b2c767f2a463aab3ec\",\"PreviousHash\":\"d9b5d942a14db7a408328b178bfb531b508b322a08264cbf189c008947512634\",\"TimeStamp\":1508236436,\"Nonce\":0}" http://127.0.0.1:8080/validate
```
```json
{"Status":true}
```
