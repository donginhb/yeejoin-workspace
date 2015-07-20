
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "httpserver-netconn.h"

#include "grlib.h"

extern const tDisplay g_sLcd240x320x16_8bit;

#if LWIP_NETCONN

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_html[] = 
        "<html><head><title>Congrats!-YEJOIN</title></head><body><h1>Welcome to our lwIP HTTP server(YEJOIN)!</h1><p>This is a small test page(亿江-北京), served by httpserver-netconn.</body></html>";

/** Serve one HTTP connection accepted in the http thread */
static void
http_server_netconn_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    err_t err;

    /* Read the data from the port, blocking if nothing yet there. 
    We assume the request (the part we care about) is in one netbuf */
    err = netconn_recv(conn, &inbuf);

    if (err == ERR_OK) {
        netbuf_data(inbuf, (void**)&buf, &buflen);

        /* Is this an HTTP GET command? (only check the first 5 chars, since
        there are other formats for GET, and we're keeping it very simple )*/
        if (buflen>=5 && buf[0]=='G' && buf[1]=='E'
            && buf[2]=='T' && buf[3]==' ' && buf[4]=='/') {
          /* Send the HTML header 
           * subtract 1 from the size, since we dont send the \0 in the string
           * NETCONN_NOCOPY: our data is const static, so no need to copy it
           */
          netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
          
          /* Send our HTML page */
          netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
        }
    }
    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);

    /* Delete the buffer (netconn_recv gives us ownership,
    so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);
}


//static struct ip_addr localip;

/** The main function, never returns! */
static void
http_server_netconn_thread(void *arg)
{
    struct netconn *conn, *newconn;
    err_t err;
    
    LWIP_UNUSED_ARG(arg);

    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);

    /* Bind to port 80 (HTTP) with default IP address */
    /*localip.addr = DEFAULT_IPADDR;
    netconn_bind(conn, &localip, 80); */ /* David, set IP */
    netconn_bind(conn, NULL, 80);

    /* Put the connection into LISTEN state */
    netconn_listen(conn);

    do {
        err = netconn_accept(conn, &newconn);

        if (err == ERR_OK) {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while(err == ERR_OK);
    LWIP_DEBUGF(HTTPD_DEBUG,
            ("http_server_netconn_thread: netconn_accept received error %d, shutting down",
            err));
    netconn_close(conn);
    netconn_delete(conn);
}

#define TCPIP_THREAD_STACKSIZE_1 (1024 * 4)

/** Initialize the HTTP server (start its thread) */
void http_server_netconn_init(void)
{
    #if (TCPIP_THREAD_STACKSIZE == 0)
    //#error "TCPIP_THREAD_STACKSIZE is 0!!"
    #endif
    if (NULL == sys_thread_new("http_server_netconn", http_server_netconn_thread, NULL, TCPIP_THREAD_STACKSIZE_1, TCPIP_THREAD_PRIO)) {
        rt_kprintf("lwip creat https server thread fail!\n\r");
    } else {
        rt_kprintf("lwip creat https server thread succ!\n\r");
    }
}


void vApplicationMallocFailedHook( void )
{
    g_sLcd240x320x16_8bit.pfnLineDrawV(NULL, 90, 100, 200, 0X1f7ff); /* tag 8, start 10 */
    rt_kprintf("malloc fail!\n\r");
}
#endif /* LWIP_NETCONN*/
