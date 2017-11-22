/*
 *  Copyright (c) 2015, Parse, LLC. All rights reserved.
 *
 *  You are hereby granted a non-exclusive, worldwide, royalty-free license to use,
 *  copy, modify, and distribute this software in source code or binary form for use
 *  in connection with the web services and APIs provided by Parse.
 *
 *  As with any software that integrates with the Parse platform, your use of
 *  this software is subject to the Parse Terms of Service
 *  [https://www.parse.com/about/terms]. This copyright notice shall be
 *  included in all copies or substantial portions of the software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 *  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "client.h"

int tcp_connect(const char *host, int port)
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;
    if(!(hp=gethostbyname(host))) {
        return -1;
    }
    memset(&addr,0,sizeof(addr));
    addr.sin_addr=*(struct in_addr*) hp->h_addr_list[0];
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);

    if((sock=socket(AF_INET,SOCK_STREAM, IPPROTO_TCP))<0) {
        return -1;
    }
    if(connect(sock,(struct sockaddr *)&addr, sizeof(addr))<0) {
        return -1;
    }
    return sock;
}

int tcp_close(int sock) {
    return close(sock);
}

int tcp_write(int sock, char* data) {
    int n_sent = 0;
    int n_bytes = strlen(data);
    int n_tmp = 0;
    while(n_sent < n_bytes && (n_tmp = send(sock, data+n_sent, n_bytes-n_sent, 0))) {
        if(n_tmp == -1){
            break;
        }
        n_sent += n_tmp;
    }


    if(n_sent < n_bytes){
        // write incomplete
        return 0;
    }

    return 1;
}

int tcp_read(int sock, char* data, int size, int timeout_sec) {
    struct timeval tv;
    fd_set receive;
    int n_recv = 0;
    int n_tmp = 0;
    int bracketCount = 0;
    int readCount = 0;
    while(n_recv < size && readCount++ < timeout_sec * 10) {
        FD_ZERO(&receive);
        FD_SET(sock, &receive);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if(select(sock + 1, &receive, NULL, NULL, &tv) > 0) {
            if(FD_ISSET(sock, &receive)) {
                // need to put select to timeout asap
                n_tmp = recv(sock, data+n_recv, size-n_recv, 0);
                if (n_tmp == -1) {
                    n_tmp = 0;
                }
                // Analyze the data to get the object asap.
                int i;
                for(i = 0; i < n_tmp; i ++) {
                    if (data[n_recv + i] == '{') bracketCount++;
                    if (data[n_recv + i] == '}') bracketCount--;
                }
                n_recv += n_tmp;
                if (n_recv > 0 && bracketCount == 0) {
                    break;
                }
            }
        }
    }
    data[n_recv] = '\0';
    return n_recv;
}

