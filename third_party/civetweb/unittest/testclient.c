#include <stdio.h>
#include <time.h>

#if defined(_WIN32) || defined(WIN32) 
#include <windows.h>
void INIT(void) {WSADATA wsaData; WSAStartup(MAKEWORD(2,2), &wsaData);}
#else
#define INIT()
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

int connect_to_server(const struct sockaddr_in * serv_addr)
{
    int sockfd;

    /* Create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    /* Connect to the server */
    if (connect(sockfd, (const sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
         perror("ERROR connecting");
         close(sockfd);
         return -2;
    }

    return sockfd;
}

int send_to_server(int conn, const char * request)
{
    int req_len = strlen(request);
    int n;

    n = write(conn, request, req_len);
    if (n < 0) {
         perror("ERROR writing to socket");
         return 0;
    }

    return (n==req_len);
}

int read_from_server(int conn)
{
    char rbuffer[1024];
    int n;
    long ret;

    n = read(conn, rbuffer, sizeof(rbuffer));
    if (n < 0) {
         perror("ERROR reading from socket");
         return 0;
    }

    if (strncmp("HTTP/1.", rbuffer, 7)) {
         perror("ERROR not a HTTP response");
         return 0;
    }

    ret = atol(rbuffer + 9);

    return ret;
}


int main(int argc, char *argv[])
{
    long portno;
    int i, con_count=1;
    time_t t1,t2,t3,t4;
    char wbuffer[256];
    int connlist[1024*65];
    int result[1024*65];
    struct hostent *server;
    struct sockaddr_in serv_addr;

    INIT();

    if (argc != 4) {
        fprintf(stderr,"Usage:\n\t%s hostname port clients\n\n", argv[0]);
        exit(0);
    }

    con_count = atol(argv[3]);
    if (con_count<1) con_count=1;
    if (con_count>1024*65) con_count=1024*65;

    portno = atol(argv[2]);
    if (portno<1l || portno>0xFFFFl) {
        fprintf(stderr, "ERROR, invalid port\n");
        exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons((short)portno);

    sprintf(wbuffer, "GET / HTTP/1.0\r\n\r\n");

    t1 = time(0);
    for (i=0;i<con_count;i++) {
        result[i] = connlist[i] = connect_to_server(&serv_addr);
    }
    t2 = time(0);
    for (i=0;i<con_count;i++) {
        if (result[i]>=0) {
            result[i] = send_to_server(connlist[i], wbuffer);
        }
    }
    t3 = time(0);
    for (i=0;i<con_count;i++) {
        if (result[i]>=0) {
            result[i] = read_from_server(connlist[i]);
        }
    }
    t4 = time(0);

    printf("\n");
    printf("conn:  %.0lf\n", difftime(t2,t1));
    printf("write: %.0lf\n", difftime(t3,t2));
    printf("read:  %.0lf\n", difftime(t4,t3));

    for (i=-10;i<1000;i++) {
        int j,cnt=0;
        for(j=0;j<con_count;j++) {
            if (result[j]==i) cnt++;
        }
        if (cnt>0) {
            printf("%5i\t%7i\n", i, cnt);
        }
    }

    return 0;
}


