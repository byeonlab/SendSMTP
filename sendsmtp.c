#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define SEND_BUF_SIZE 0xFFFF
#define RECV_BUF_SIZE 0xFFF

static void sendmail_write(
                const int  sock,
                const char *str,
                const char *arg
            ) {
    char send_buf[SEND_BUF_SIZE];

    if (arg != NULL)
        snprintf(send_buf, SEND_BUF_SIZE, str, arg);        
    else
        snprintf(send_buf, SEND_BUF_SIZE, str);

    send(sock, send_buf, strlen(send_buf), 0);
}

static int sendmail(
                const char *from,
                const char *to,
                const char *subject,
                const char *body,
                const char *smtp_server,
                const char *smtp_port
            ) {
    struct hostent *host;   
    struct sockaddr_in saddr_in;
    int sock = 0;
    int port;
    char recv_buf[RECV_BUF_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    host = gethostbyname(smtp_server);
    port = atoi(smtp_port);
   
    saddr_in.sin_family      = AF_INET;
    saddr_in.sin_port        = htons((u_short)port);
    saddr_in.sin_addr.s_addr = 0;

    memcpy((char*)&(saddr_in.sin_addr), host->h_addr, host->h_length);

    if (connect(sock, (struct sockaddr*)&saddr_in, sizeof(saddr_in)) == -1) {
       return -2;
    }
    
    recv(sock, recv_buf, RECV_BUF_SIZE, 0);               // 220 
    sendmail_write(sock, "EHLO localhost\r\n",  NULL);    // CLIENT HELLO
    recv(sock, recv_buf, RECV_BUF_SIZE, 0);               // SERVER HELLO
    sendmail_write(sock, "MAIL FROM: <%s>\r\n", from);    // sender@from.com
    recv(sock, recv_buf, RECV_BUF_SIZE, 0);               // Sender OK
    sendmail_write(sock, "RCPT TO: <%s>\r\n",   to);      // RCPT TO: recipient@to.com
    recv(sock, recv_buf, RECV_BUF_SIZE, 0);               // Recipient OK
    sendmail_write(sock, "DATA\r\n",            NULL);    // begin data
    recv(sock, recv_buf, RECV_BUF_SIZE, 0);               // 354 Start mail input
                                                       
    //Mail                                                        // ---- Mail Format ----
    sendmail_write(sock, "From: %s\r\n",                from);    // From: sender@from.com
    sendmail_write(sock, "To: %s\r\n",                  to);      // To: recipient@to.com
    sendmail_write(sock, "Subject: %s\r\n",             subject); // Subject: Hello World!
    sendmail_write(sock, "Content-Type: text/html\r\n", NULL);    // Content-Type: text/html
    sendmail_write(sock, "MIME-Version: 1.0\r\n",       NULL);    // MIME-Version: 1.0
    sendmail_write(sock, "\r\n",                        NULL);    // (empty line)
    sendmail_write(sock, "%s\r\n",                      body);    // This is mail body.
    sendmail_write(sock, ".\r\n",                       NULL);    // .
    sendmail_write(sock, "quit\r\n",                    NULL);    // quit

    close(sock);

    return 0;
}


int main(int argc, char *argv[]) {
    if (argc != 7){
        fprintf(stderr, "usage: sendsmtp [from] [to] [subject] [body] [smtp server] [smtp port]\n");
	return -1;
    }
    
    int ret = sendmail(
	argv[1], /* from */
	argv[2], /* to */
	argv[3], /* subject */
	argv[4], /* body */
	argv[5], /* smtp server */
	argv[6]  /* smtp port */
    );

    if (ret != 0)
        fprintf(stderr, "Failed to send mail (code: %d).\n", ret);
    else
        fprintf(stdout, "Mail successfully sent.\n");

    return ret;
}
