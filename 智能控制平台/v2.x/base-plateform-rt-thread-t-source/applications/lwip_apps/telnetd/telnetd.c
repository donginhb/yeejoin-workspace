/*
 * telnetd.c
 */
/**
  ******************************************************************************
  * @file    helloworld.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A hello world example based on a Telnet connection
  *          The application works as a server which wait for the client request
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "telnetd.h"
#include "lwip/tcp.h"
#include <string.h>

#include <syscfgdata.h>
#include <rtdef.h>

#include <board.h>

#include <finsh.h>
#include <shell.h>

#include <base_ds.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define LOGIN_NAME "login:"
#define PASSWORD   "pw:"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



#define ISO_nl 0x0a
#define ISO_cr 0x0d
#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254

#define TELNET_ECHO                 1
#define TELNET_SUPPRESS_GO_AHEAD    3



extern rt_err_t rt_telnetio_register(rt_device_t device, const char* name, rt_uint32_t flag, struct tcp_pcb *pcb);
extern struct telnetio_dev *telnetio_dev_creat(void);
extern void telnetio_dev_delete(struct telnetio_dev *teldev);

/* Private function prototypes -----------------------------------------------*/
static err_t telnetd_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t telnetd_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static void telnetd_conn_err(void *arg, err_t err);

static void write_iac_cmd2tx_buf(struct telnetio_dev *teldev, u8_t option, u8_t value);
static void proc_rx_data(struct tcp_pcb * pcb, struct telnetio_dev *teldev, struct pbuf *p);
static void proc_rx_byte(struct tcp_pcb * pcb, struct telnetio_dev *teldev, unsigned char ch);





/**
  * @brief  Initialize the hello application  
  * @param  None 
  * @retval None 
  */
void telnetd_init(void)
{
    struct tcp_pcb *pcb;	            		

    /* Create a new TCP control block  */
    pcb = tcp_new();	                		 	

    /* Assign to the new pcb a local IP address and a port number */
    /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
    tcp_bind(pcb, IP_ADDR_ANY, 23);       

    /* Set the connection to the LISTEN state */
    pcb = tcp_listen(pcb);				

    /* Specify the function to be called when a connection is established */	
    tcp_accept(pcb, telnetd_accept);   

    return;
}


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  This function when the Telnet connection is established
  * @param  arg  user supplied argument 
  * @param  pcb	 the tcp_pcb which accepted the connection
  * @param  err	 error value
  * @retval ERR_OK
  */
static err_t telnetd_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
    struct telnetio_dev *p;
   
    p = telnetio_dev_creat();
    if (NULL == p) {
        return ERR_MEM;
    } else {
        /* Tell LwIP to associate this structure with this connection. */
        tcp_arg(pcb, p);

        /* p所指向的区域已清零 */
        /* 这里只是简单的处理 */
        p->dev_name[0] = 't';
        p->dev_name[1] = 'e';
        p->dev_name[2] = 'l';
        p->dev_name[3] = 'n';
        p->dev_name[4] = '0' + telnetio_dev_no_get();
        p->dev_name[5] = '\0';
        rt_telnetio_register(&p->dev, p->dev_name,  0, pcb);

        //rt_device_open(&p->dev, 0);
    	finsh_set_device(p->dev_name);
    	rt_console_set_device(p->dev_name);
    }

    /* Configure LwIP to use our call back functions. */
    tcp_err(pcb, telnetd_conn_err);
    tcp_recv(pcb, telnetd_recv);

    /* !!!! 发送telnet选项命令 */
    
    /* Send out the first message */  
    /* tcp_write(pcb, GREETING, strlen(GREETING), 1);  */
    /* send out the login message */
    tcp_write(pcb, LOGIN_NAME, strlen(LOGIN_NAME), TCP_WRITE_FLAG_COPY);
    p->state = TELS_LOGIN_NAME;

    return ERR_OK;
}



/**
  * @brief  Called when a data is received on the telnet connection
  * @param  arg	the user argument
  * @param  pcb	the tcp_pcb that has received the data
  * @param  p	the packet buffer, 存放接收到的数据
  * @param  err	the error value linked with the received data
  * @retval error value
  */
static err_t telnetd_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct telnetio_dev *teldev = arg;

    /* We perform here any necessary processing on the pbuf */
    if (p != NULL) {        
        /* We call this function to tell the LwIp that we have processed the data */
        /* This lets the stack advertise a larger window, so more data can be received*/
        tcp_recved(pcb, p->tot_len);

        /* Check the name if NULL, no data passed, return withh illegal argument error */
        if(NULL == teldev) {
            pbuf_free(p);
            return ERR_ARG;
        }

        proc_rx_data(pcb, teldev, p);

        /* End of processing, we free the pbuf */
        pbuf_free(p);
    } else if (err == ERR_OK) {
        /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
        /* We free the allocated memory and we close the connection */
        //rt_device_close(&teldev->dev);
    	finsh_set_device(CONSOLE_DEVICE);
    	rt_console_set_device(CONSOLE_DEVICE);

        rt_device_unregister(&teldev->dev);
        telnetio_dev_delete(teldev);

        return tcp_close(pcb);
    }

    return ERR_OK;
}



/**
  * @brief  This function is called when an error occurs on the connection 
  * @param  arg
  * @parm   err
  * @retval None 
  */
static void telnetd_conn_err(void *arg, err_t err)
{
    struct telnetio_dev *teldev = arg;

	finsh_set_device(CONSOLE_DEVICE);
	rt_console_set_device(CONSOLE_DEVICE);

    rt_device_unregister(&teldev->dev);
    telnetio_dev_delete(teldev);

    return;
}

/*
 *  Telnet will/wont/do/dont formatting
 */
static void write_iac_cmd2tx_buf(struct telnetio_dev *teldev, u8_t option, u8_t value)
{
    unsigned char *p;
    int i;

    i = teldev->iac_index;
    if (i >= 64-4)
        return;

    p = teldev->iac_buf;

    p[i++] = TELNET_IAC;
    p[i++] = option;
    p[i++] = value;

    teldev->iac_index = i;

    return;
}


static void proc_rx_data(struct tcp_pcb * pcb, struct telnetio_dev *teldev, struct pbuf *p)
{
    struct pbuf *q;
    unsigned char *c;
    int num, i;

    rt_sem_take(teldev->rx_rb_buf.rw_sem, RT_WAITING_FOREVER);
    for(q=p; NULL!=q; q=q->next) {
        c = q->payload;
        for(i=0; i<q->len; i++, c++)
            proc_rx_byte(pcb, teldev, *c);
    }
    rt_sem_release(teldev->rx_rb_buf.rw_sem);

    num = teldev->iac_index;
    if (0 != num) {
        tcp_write(pcb, teldev->iac_buf, num, TCP_WRITE_FLAG_COPY);
        teldev->iac_index = 0;
    }
    
    return;
}

static void proc_rx_byte(struct tcp_pcb * pcb, struct telnetio_dev *teldev, unsigned char ch)
{
    int num, i;
    unsigned char ch1;
    
//printf_syn("line:%u, iac_s:%d, state:%d\r\n", __LINE__, teldev->iac_state, teldev->state);
    if (0 == teldev->iac_state) {
        if (TELNET_IAC == ch) {
            teldev->iac_state = TELS_IAC;
            return;
        }
        if ('\n' != ch) {
            rb_write(&teldev->rx_rb_buf, &ch, 1);

            if ('\r' != ch)
                return;
        } else {
            return;
        }

        switch (teldev->state) {
        case TELS_LOGIN_NAME:
            do {
                if ((0==rb_first_read_byte_pry(&teldev->rx_rb_buf, &ch1))
                        && '\r'==ch1)
                    rb_first_read_byte_drop(&teldev->rx_rb_buf);
                else 
                    break;
            }while(1);
            num = rb_get_used_bytes_num(&teldev->rx_rb_buf) - 1;

            i = MIN(num, USR_NAME_LEN_MAX);
            rb_read(&teldev->rx_rb_buf, teldev->usrpw.usr, i);
            teldev->usrpw.usr[i] = '\0';

            rb_cleanup(&teldev->rx_rb_buf);

//    printf_syn("line:%u, input:%s\r\n", __LINE__, teldev->usrpw.usr);
            tcp_write(pcb, PASSWORD, strlen(PASSWORD), TCP_WRITE_FLAG_COPY);
            //tcp_output(pcb);
            teldev->state = TELS_LOGIN_PW;

            write_iac_cmd2tx_buf(teldev, TELNET_WONT, TELNET_SUPPRESS_GO_AHEAD);
            write_iac_cmd2tx_buf(teldev, TELNET_WILL, TELNET_ECHO);
            break;

        case TELS_LOGIN_PW:
            do {
                if ((0==rb_first_read_byte_pry(&teldev->rx_rb_buf, &ch1))
                        && '\r'==ch1)
                    rb_first_read_byte_drop(&teldev->rx_rb_buf);
                else 
                    break;
            }while(1);
            num = rb_get_used_bytes_num(&teldev->rx_rb_buf) - 1;

            i = MIN(num, PW_LEN_MAX);
            rb_read(&teldev->rx_rb_buf, teldev->usrpw.pw, i);
            teldev->usrpw.pw[i] = '\0';

            rb_cleanup(&teldev->rx_rb_buf);

    //printf_syn("line:%u, usr:%s, pw:%s\r\n", __LINE__, teldev->usrpw.usr, teldev->usrpw.pw);
            if (is_usr_pw_matching(teldev->usrpw.usr, teldev->usrpw.pw)) {
                teldev->state = TELS_NORMAL;
                teldev->iac_state = 0;

                rb_write(&teldev->rx_rb_buf, "\r", 1);
                rt_device_read(&teldev->dev, 0, NULL, 0);                
            } else {
                tcp_write(pcb, LOGIN_NAME, strlen(LOGIN_NAME), TCP_WRITE_FLAG_COPY);
                teldev->state = TELS_LOGIN_NAME;
            }

            write_iac_cmd2tx_buf(teldev, TELNET_WONT, TELNET_SUPPRESS_GO_AHEAD);
            write_iac_cmd2tx_buf(teldev, TELNET_WONT, TELNET_ECHO);
            break;

        case TELS_NORMAL :
            rt_device_read(&teldev->dev, 0, NULL, 0);
            break;

        case TELS_CLOSE :
        case TELS_LOGOUT:
        default:
            break;
        }
    } else {
        switch (teldev->iac_state) {
        case TELS_IAC :
            switch (ch) {
            case TELNET_WILL :
              teldev->iac_state = TELS_WILL;
              break;

            case TELNET_WONT :
              teldev->iac_state = TELS_WONT;
              break;

            case TELNET_DO :
              teldev->iac_state = TELS_DO;
              break;

            case TELNET_DONT :
              teldev->iac_state = TELS_DONT;
              break;

            case TELNET_IAC:
            default :
              teldev->iac_state = 0;//TELS_NORMAL;
              break;
            }
            break;

        case TELS_WILL : /* Reply with a DONT */
            write_iac_cmd2tx_buf (teldev, TELNET_DONT, ch);
            teldev->iac_state = 0;
            break;

        case TELS_WONT : /* Reply with a DONT */
            write_iac_cmd2tx_buf (teldev, TELNET_DONT, ch);
            teldev->iac_state = 0;
            break;

        case TELS_DO : /* Reply with a WONT */
            if ((TELS_LOGIN_PW == teldev->state) && TELNET_ECHO==ch)
                write_iac_cmd2tx_buf (teldev, TELNET_WILL, ch);
            else
                write_iac_cmd2tx_buf (teldev, TELNET_WONT, ch);
            teldev->iac_state = 0;
            break;

        case TELS_DONT : /* Reply with a WONT */
            write_iac_cmd2tx_buf (teldev, TELNET_WONT, ch);
            teldev->iac_state = 0;
            break;
        default:
            teldev->iac_state = 0;
            break;
        }
    }
}

