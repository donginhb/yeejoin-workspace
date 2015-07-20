#include <rtthread.h>
#include <dfs_posix.h>
#include <lwip/sockets.h>

#include <finsh.h>

#define TFTP_PORT	69
/* opcode */
#define TFTP_RRQ		1 	/* read request */
#define TFTP_WRQ		2	/* write request */
#define TFTP_DATA		3	/* data */
#define TFTP_ACK		4	/* ACK */
#define TFTP_ERROR		5	/* error */

rt_uint8_t tftp_buffer[512 + 4];


/************************tftp client **********************************/
/**********************************************************************/
void tftp_get(const char* host, const char* dir, const char* filename)
{
	int fd, sock_fd, sock_opt;
	
	uint16_t *p, tftp_cur_blk; 
	struct sockaddr_in tftp_addr, from_addr;
	rt_uint32_t length;
	socklen_t fromlen;
#if 0
	/* make local file name */
	rt_snprintf((char*)tftp_buffer, sizeof(tftp_buffer),
		"%s/%s", dir, filename);

	/* open local file for write */
	fd = open((char*)tftp_buffer, O_RDWR | O_CREAT, 0);
	if (fd < 0)
	{
		printf_syn("can't open local filename\n");
		return;
	}
#endif
	/* connect to tftp server */
    inet_aton(host, (struct in_addr*)&(tftp_addr.sin_addr));
    tftp_addr.sin_family = AF_INET;
    tftp_addr.sin_port = htons(TFTP_PORT);
    
    sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (sock_fd < 0)
	{
	    close(fd);
	    printf_syn("can't create a socket\n");
	    return ;
	}
	
	/* set socket option */
	sock_opt = 5000; /* 5 seconds */
	lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));
	
	/********************** make tftp request *************************/
	
	/*将filename写入buffer[2]中，0，数据长度自加1，
	  将octet模式写入buffer中，0，数据长度自加1*/
	tftp_buffer[0] = 0;			/* opcode */
	tftp_buffer[1] = TFTP_RRQ; 	/* RRQ */
	length = rt_sprintf((char *)&tftp_buffer[2], "%s", filename) + 2;
	tftp_buffer[length] = 0; length ++;
	length += rt_sprintf((char*)&tftp_buffer[length], "%s", "octet");
	tftp_buffer[length] = 0; length ++;
	
	fromlen = sizeof(struct sockaddr_in);
	/******************************************************************/
	
	/* send request */	
	lwip_sendto(sock_fd, tftp_buffer, length, 0, 
		(struct sockaddr *)&tftp_addr, fromlen);
	tftp_cur_blk = 1;

	do
	{	
		length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
			(struct sockaddr *)&from_addr, &fromlen);
			
		//printf_syn("recv data:%s\n", (char*)&tftp_buffer[4]);
		
		p = (rt_uint16_t *) & tftp_buffer[2];//这段中保存着块编号
		//判断当前收到的数据是否为我想要的数据
		if(*p == lwip_htons(tftp_cur_blk))
		{
			if (length > 0)
			{
				//write(fd, (char*)&tftp_buffer[4], length - 4);
				//printf_syn("#");
				tftp_buffer[length] = 0;
				printf_syn("recv data:%s\n", (char*)&tftp_buffer[4]);
				/* make ACK */			
				tftp_buffer[0] = 0; 
				tftp_buffer[1] = TFTP_ACK; /* opcode */				
				/* send ACK */
				lwip_sendto(sock_fd, tftp_buffer, 4, 0, (struct sockaddr *)&from_addr, fromlen);				
			}
			++tftp_cur_blk;
		}else{}
		
	} while (length == 516);

	if (length == 0) printf_syn("timeout\n");
	else printf_syn("done\n");

	close(fd);
	lwip_close(sock_fd);
}
FINSH_FUNCTION_EXPORT(tftp_get, get file from tftp server);
#if 0
void tftp_put(const char* host, const char* dir, const char* filename)
{
	int fd, sock_fd, sock_opt;
	struct sockaddr_in tftp_addr, from_addr;
	rt_uint32_t length, block_number = 0;
	socklen_t fromlen;

	#if 0
		/* make local file name */
		rt_snprintf((char*)tftp_buffer, sizeof(tftp_buffer),
			"%s/%s", dir, filename);

		/* open local file for write */
		fd = open((char*)tftp_buffer, O_RDONLY, 0);
		if (fd < 0)
		{
			printf_syn("can't open local filename\n");
			return;
		}
	#endif

	/* connect to tftp server */
    inet_aton(host, (struct in_addr*)&(tftp_addr.sin_addr));
    tftp_addr.sin_family = AF_INET;
    tftp_addr.sin_port = htons(TFTP_PORT);

    sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if (sock_fd < 0)
	{
	    close(fd);
	    printf_syn("can't create a socket\n");
	    return ;
	}

	/* set socket option */
	sock_opt = 5000; /* 5 seconds */
	lwip_setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_opt, sizeof(sock_opt));

	/* make tftp request */
	tftp_buffer[0] = 0;			/* opcode */
	tftp_buffer[1] = TFTP_WRQ; 	/* WRQ */
	length = rt_sprintf((char *)&tftp_buffer[2], "%s", filename) + 2;
	tftp_buffer[length] = 0; length ++;
	length += rt_sprintf((char*)&tftp_buffer[length], "%s", "octet");
	tftp_buffer[length] = 0; length ++;

	fromlen = sizeof(struct sockaddr_in);
	
	/* send request */	
	lwip_sendto(sock_fd, tftp_buffer, length, 0, 
		(struct sockaddr *)&tftp_addr, fromlen);

	/* wait ACK 0 */	
	length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
		(struct sockaddr *)&from_addr, &fromlen);
	if (!(tftp_buffer[0] == 0 &&
		tftp_buffer[1] == TFTP_ACK &&
		tftp_buffer[2] == 0 &&
		tftp_buffer[3] == 0))
	{
		printf_syn("tftp server error\n");
		close(fd);
		return;
	}

	block_number = 1;
	
	while (1)
	{
		length = read(fd, (char*)&tftp_buffer[4], 512);
		if (length > 0)
		{
			/* make opcode and block number */
			tftp_buffer[0] = 0; tftp_buffer[1] = TFTP_DATA;
			tftp_buffer[2] = (block_number >> 8) & 0xff;
			tftp_buffer[3] = block_number & 0xff;

			lwip_sendto(sock_fd, tftp_buffer, length + 4, 0, 
				(struct sockaddr *)&from_addr, fromlen);
		}
		else
		{
			printf_syn("done\n");
			break; /* no data yet */
		}

		/* receive ack */
		length = lwip_recvfrom(sock_fd, tftp_buffer, sizeof(tftp_buffer), 0, 
			(struct sockaddr *)&from_addr, &fromlen);
		if (length > 0)
		{
			if ((tftp_buffer[0] == 0 &&
				tftp_buffer[1] == TFTP_ACK &&
				tftp_buffer[2] == (block_number >> 8) & 0xff) &&
				tftp_buffer[3] == (block_number & 0xff))
			{
				block_number ++;
				printf_syn("#");
			}
			else 
			{
				printf_syn("server respondes with an error\n");
				break;
			}
		}
		else if (length == 0)
		{
			printf_syn("server timeout\n");
			break;
		}
	}

	close(fd);
	lwip_close(sock_fd);
}
FINSH_FUNCTION_EXPORT(tftp_put, put file to tftp server);
#endif
