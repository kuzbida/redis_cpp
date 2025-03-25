# Build Server
```
g++ -std=c++17 -g server.cpp src/avl.cpp src/hashtable.cpp src/zset.cpp -o server
```

# Build Client
```
g++ -std=c++17 client.cpp -o client
```

# Client available commands
```
./client get msg
./client set msg 1
./client zscore asdf n1
./client zadd zset 1 n1
./client zadd zset 1 n1
```