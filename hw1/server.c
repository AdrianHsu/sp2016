#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define ERR_EXIT(a) { perror(a); exit(1); }

typedef struct {
  char hostname[512];  // server's hostname
  unsigned short port;  // port to listen
  int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
  char host[512];  // client's host
  int conn_fd;  // fd to talk with client
  char buf[512];  // data sent by/to client
  size_t buf_len;  // bytes used by buf
  // you don't need to change this.
  char* filename;  // filename set in header, end with '\0'.
  int header_done;  // used by handle_read to know if the header is read or not.
  int file_fd;
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_header = "ACCEPT\n";
const char* reject_header = "REJECT\n";

// Forwards

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

static int handle_read(request* reqP);
// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error


#define SVR_IP "127.0.0.1"
#define SVR_PORT 8888
#define BUF_SIZE 1024

int main(int argc, char** argv) {
  int i, ret;

  struct sockaddr_in cliaddr;  // used by accept()
  int clilen;

  int conn_fd;  // fd for a new connection with client
  int file_fd;  // fd for file that we open for reading
  char buf[512];
  int buf_len;
  const char *a[500];
  a[0] = "";
  // Parse args.
  if (argc != 2) {
    fprintf(stderr, "usage: %s [port]\n", argv[0]);
    exit(1);
  }

  // Initialize server
  init_server((unsigned short) atoi(argv[1]));

  // Get file descripter table size and initize request table
  maxfd = getdtablesize();
  requestP = (request*) malloc(sizeof(request) * maxfd);
  if (requestP == NULL) {
    ERR_EXIT("out of memory allocating all requests");
  }
  for (i = 0; i < maxfd; i++) {
    init_request(&requestP[i]);
  }
  requestP[svr.listen_fd].conn_fd = svr.listen_fd;
  strcpy(requestP[svr.listen_fd].host, svr.hostname);
  // Loop for handling connections
  fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

  fd_set master, read_fds, write_fds;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_SET(svr.listen_fd, &master);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1;

#ifdef READ_SERVER
  struct flock rlock;
  rlock.l_type = F_RDLCK;
  rlock.l_whence = SEEK_SET;  
  rlock.l_start = 0;  
  rlock.l_len = 0;  
  //rlock.l_pid -1; //getpid  
#endif
#ifndef READ_SERVER
  struct flock wlock;
  wlock.l_type = F_WRLCK;
  wlock.l_whence = SEEK_SET;  
  wlock.l_start = 0;  
  wlock.l_len = 0;  
  //wlock.l_pid -2;
#endif

  while (1) {
    // TODO: Add IO multiplexing
    // Check new connection
    memcpy(&read_fds, &master, sizeof(master)); 
    memcpy(&write_fds, &master, sizeof(master)); //they are the same fds
    // int result = select(maxfd + 1, &read_fds, NULL, NULL, &timeout);//read_fds, write_fds repeat

    // use select() to find which file descriptors are ready.
    int result = select(maxfd + 1, &read_fds, NULL, NULL, NULL);//read_fds, write_fds repeat

    if(result <= 0) // no available fds
      continue; // return at once
    
    int _result = 0;
    if(FD_ISSET(svr.listen_fd, &read_fds)) {
      clilen = sizeof(cliaddr);
      // accept() new connections if there's any. 
      conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
      if (conn_fd < 0) {
        if (errno == EINTR || errno == EAGAIN) continue;  // try again
        if (errno == ENFILE) {
          (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
          continue;
        }
        ERR_EXIT("accept")
      }
      requestP[conn_fd].conn_fd = conn_fd; // this connection's fd, denotes client address
      requestP[conn_fd].file_fd = file_fd = -1; // r/w file

      FD_SET(conn_fd, &master);
      strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
      fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
      _result++;
    }
    
    int i = svr.listen_fd + 1; // current id
    //for each request with file descriptor ready
    for(; _result < result; _result++) {

      while(!FD_ISSET(i, &read_fds) && i < maxfd)// same as write_fds 
        i++;
      if(i == maxfd)
        break;

#ifdef READ_SERVER
      //use handle_read to read from the request
      ret = handle_read(&requestP[i]);
      if (ret < 0) {
        fprintf(stderr, "bad request from %s\n", requestP[i].host);
        continue;
      }
      // requestP[i]->filename is guaranteed to be successfully set.
      
      // if this is the first time read of the request
      if (requestP[i].file_fd == -1) {

        // TODO: Add lock
        // TODO: check if the request should be rejected.
        // check lock and obtain lock for the file, reject when can't lock
        // responds to client that the request is accepted.
        // open the file descriptor for the file
        file_fd = open(requestP[i].filename, O_RDONLY, 0);
        requestP[i].file_fd = file_fd;
        int same_name = 0; 
        fprintf(stderr, "1 = %s, 2 = %s \n", requestP[i].filename, a[0]);
        if(strcmp(a[0],requestP[i].filename) == 0)
          same_name = 1;
        
        if((fcntl(requestP[i].file_fd, F_SETLKW, &rlock) != -1) && !same_name) { 
          // locked
          fprintf(stderr, "Opening file [%s]\n", requestP[i].filename);
          write(requestP[i].conn_fd, accept_header, sizeof(accept_header));
          //continue;
          a[0] = requestP[i].filename;
        } else {
          fprintf(stderr, "Reject reading file [%s]\n", requestP[i].filename);
          write(requestP[i].conn_fd, reject_header, sizeof(reject_header));
          FD_CLR(i, &master); //delete this already-read file from master & read_fds 
          if (file_fd >= 0) close(file_fd);
          close(requestP[i].conn_fd);
          free_request(&requestP[i]);
          a[0] = "";
          //continue;
          break;
        }
      //} else {
        if (ret == 0) {
          break;
        }
        while (1) {
          ret = read(requestP[i].file_fd, buf, sizeof(buf));
          if (ret < 0) {
            fprintf(stderr, "Error when reading file %s\n", requestP[i].filename);
            break;
          } else if (ret == 0) break;
          write(requestP[i].conn_fd, buf, ret);
        }
        FD_CLR(i, &master); //delete this already-read file from master & read_fds
        fprintf(stderr, "Done reading file [%s]\n", requestP[i].filename);
      }
      if (file_fd >= 0) close(file_fd);
      close(requestP[i].conn_fd);
      free_request(&requestP[i]);
      a[0] = "";
      fprintf(stderr, "haha\n");
#endif
#ifndef READ_SERVER
      //use handle_read to read from the request
      ret = handle_read(&requestP[i]);
      if (ret < 0) {
        fprintf(stderr, "bad request from %s\n", requestP[i].host);
        //continue;
      }
      fprintf(stderr, "???requestP[i].file_fd, i is: %d, %d\n",requestP[i].file_fd, i);
      if (requestP[i].file_fd == -1) {
        // TODO: Add lock
        // TODO: check if the request should be rejected.
        // requestP[i]->filename is guaranteed to be successfully set.
        file_fd = open(requestP[i].filename, O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        requestP[i].file_fd = file_fd;
        
        int same_name = 0; 
        fprintf(stderr, "1 = %s, 2 = %s \n", requestP[i].filename, a[0]);
        if(strcmp(a[0],requestP[i].filename) == 0)
          same_name = 1;
        fprintf(stderr, "requestP[i].file_fd, i is: %d, %d\n",requestP[i].file_fd, i);
        if((fcntl(requestP[i].file_fd, F_SETLKW, &wlock) != -1) && !same_name) { 
          // open the file here.
          fprintf(stderr, "Opening file [%s]\n", requestP[i].filename);
          fprintf(stderr, "Accept writing file [%s]\n", requestP[i].filename);
          write(requestP[i].conn_fd, accept_header, sizeof(accept_header));
          a[0] = requestP[i].filename;
        } else {
          //perror("error\n");
          fprintf(stderr, "Reject writing file [%s]\n", requestP[i].filename);
          write(requestP[i].conn_fd, reject_header, sizeof(reject_header));
          FD_CLR(i, &master); //delete this already-read file from master & read_fds 
          if (file_fd >= 0) close(file_fd);
          close(requestP[i].conn_fd);
          free_request(&requestP[i]);
          //continue;
          a[0] = "";
          break;
        }
      }
      fprintf(stderr, "==0\n");
      if (ret == 0) {
        //break;
        //continue;
      } else { 
        fprintf(stderr, "==1\n");
        write(requestP[i].file_fd, requestP[i].buf, requestP[i].buf_len);
        fprintf(stderr, "==2\n");
      }
      fprintf(stderr, "hihi\n");
      if(ret <= 0) {
        FD_CLR(i, &master); //delete this already-read file from master & write_fds
        fprintf(stderr, "Done writing file [%s]\n", requestP[i].filename);
        if (file_fd >= 0) close(file_fd);
        close(requestP[i].conn_fd);
        free_request(&requestP[i]);
        fprintf(stderr, "haha\n");
        a[0] = "";
      }
      

#endif
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
  }
  free(requestP);
  return 0;
}


// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void* e_malloc(size_t size);


static void init_request(request* reqP) {
  reqP->conn_fd = -1;
  reqP->buf_len = 0;
  reqP->filename = NULL;
  reqP->header_done = 0;
}

static void free_request(request* reqP) {
  if (reqP->filename != NULL) {
    free(reqP->filename);
    reqP->filename = NULL;
  }
  init_request(reqP);
}

// return 0: socket ended, request done.
// return 1: success, message (without header) got this time is in reqP->buf with reqP->buf_len bytes. read more until got <= 0.
// It's guaranteed that the header would be correctly set after the first read.
// error code:
// -1: client connection error
static int handle_read(request* reqP) {
  int r;
  char buf[512];
  // Read in request from client
  r = read(reqP->conn_fd, buf, sizeof(buf));
  
  if (r < 0) return -1;
  if (r == 0) return 0;
  if (reqP->header_done == 0) {
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    // be careful that in Windows, line ends with \015\012
    if (p1 == NULL) {
      p1 = strstr(buf, "\012");
      newline_len = 1;
      if (p1 == NULL) {
        // This would not happen in testing, but you can fix this if you want.
        ERR_EXIT("header not complete in first read...");
      }
    }
    size_t len = p1 - buf + 1;
    reqP->filename = (char*)e_malloc(len);
    memmove(reqP->filename, buf, len);
    reqP->filename[len - 1] = '\0';
    p1 += newline_len;
    reqP->buf_len = r - (p1 - buf);
    memmove(reqP->buf, p1, reqP->buf_len);
    reqP->header_done = 1;
  } else {
    reqP->buf_len = r;
    memmove(reqP->buf, buf, r);
  }
  return 1;
}

static void init_server(unsigned short port) {
  struct sockaddr_in servaddr;
  int tmp;

  gethostname(svr.hostname, sizeof(svr.hostname));
  svr.port = port;

  svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (svr.listen_fd < 0) ERR_EXIT("socket");

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  tmp = 1;
  if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
    ERR_EXIT("setsockopt");
  }
  if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    ERR_EXIT("bind");
  }
  if (listen(svr.listen_fd, 1024) < 0) {
    ERR_EXIT("listen");
  }
}

static void* e_malloc(size_t size) {
  void* ptr;

  ptr = malloc(size);
  if (ptr == NULL) ERR_EXIT("out of memory");
  return ptr;
}

