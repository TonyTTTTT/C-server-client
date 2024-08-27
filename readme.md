# Readme
## Requirement
![image](https://hackmd.io/_uploads/H1AjOh9sA.png)



### Design
- Client與Server間的通訊
    - 使用TCP socket
    - 選擇的原因為可以簡單地從同台machine上不同process專換成不同machine上不同process間的溝通，其更貼近現實使用情境


- Execute Flow
    - Server waiting for client request
    - Client先找出所有的targeted files的path，把他們存在一個array of strings中
    - Client一次傳一個path給Server，Server收到path後隨即開始統計word occurrence
    - Client送完所有paths後關閉socket，Server因此知道所有paths都已送達
    - Server print統計結果
    - 回第一步


- Server Parallel處理方式
    - 一次只處理一個file，將files中的內容平均分給所有threads處理
        - 以此種方式平行處理，可以確保各個thread的loading是幾乎一樣的


- 統計時用unordered_map以獲得更好的效率，output時轉換成map以增加可讀性(ordered)


- 達成syncronization關鍵的code (in *word_occur.cpp*)
![image](https://hackmd.io/_uploads/Hk8Kr8coA.png)
    - Syncronization problem when writing into unordered_map: 不同threads同時要寫入同個key對應位址的情形


## How to run
- working in root dir
### Build
- build client
    - `gcc ./src/client.c -o ./client`
- build server
    - `gcc ./src/server.c -c -fopenmp -o ./out/server.o`
    - `g++ ./src/word_occur.cpp -c -fopenmp -o ./out/word_occur.o`
    - `g++ ./out/server.o ./out/word_occur.o -fopenmp -o ./server`
### Run
1. run server
    - `./server 5`
    ![image](https://hackmd.io/_uploads/rJRLS25i0.png)

3. run client
    - `./client ./directory_big 1724050390`
    ![image](https://hackmd.io/_uploads/Bk4pShqsA.png)
    ![image](https://hackmd.io/_uploads/r1rMBhcoC.png)



## Complexity
### Client
- N = ./directory_big中files的數量
#### Time Complexity: $O(N * average\ len\ of\ paths)$
- 主要的時間花在***traverse & find targeted file***: $O(N*(operation\ about\ each\ file))$
    - operation about each file中我認為是strcpy()和strcat()最花時間，需要$O(len(path))$
    - 因此Time Complexity about ***traverse ./directory_big並找出file***: $O(N * average\ len\ of\ paths)$
- 後續時間為等待Server處理，不計在Client的time complexity中
#### Space Complexity: $O(N * average\ len\ of\ paths)$
- 唯一會vaiable size的data就是存targeted files的array of strings，又worst case為所有files都是targeted file所以Space Complexity也是$O(N * average\ len\ of\ paths)$


### Server
- N = number of targeted files
- p = number of threads
#### Time Complexity: $O((N * average\ words\ in\ file) / p + syncronization\ overhead)$
- 最花時間的應該是統計時，針對map的操作
- 針對N個files中每個word，都要至少做一次map的操作: $O((N * average\ words\ in\ file * map\ operation) / p)$
- $map\ operation$
    - 因為使用unordered_map，所以大部分$map\ operation = O(1)$
    - 但因為parallel的統計，可能會遇到不同threads同時要寫入同個key對應位址的情形
        - syncronization問題，遇到時要保證一次只有一個thread寫入，因此performance會下降 ($syncronization\ overhead$)
        - 以total來看，還要加上$syncronization\ overhead$
- 所以Time Complexity: $O((N * average\ words\ in\ file) / p + syncronization\ overhead)$



#### Space Complexity: $O(N * average\ words\ in\ file)$
- 最佔空間的是unordered_map<string, int>
- worst case為沒有一個字出現兩次，所以有多少個字就需要多少空間，又總共有$N*average\ words\ in\ file$個字，所以Space Complexity: $O(N * average\ words\ in\ file)$


## Notice
- server使用port=48763，若該port已被使用將無法正常運行
- if `static bool debug = true;` in *util.h*
    - Also counting occurrence in serial and compare with parallel regarding time consuming
    ![image](https://hackmd.io/_uploads/Skz-dn5iR.png)
        - 文字檔足夠大的時候，parallel才會比serial快
    - More information will be print for analysis