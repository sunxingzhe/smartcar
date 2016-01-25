#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <poll.h>
#include <sys/timerfd.h>

#include <iostream>

#include "pub.h"
#include "common.h"

#include "xingzhe.h"

typedef int(xingzhe::*XZ_MSG_HANDLER)(void *);

struct pollfd fdset[64];
XZ_MSG_HANDLER msg_handler[64];

void *commu_proc(void *data);

xingzhe::xingzhe()
{
    init();
}

xingzhe::xingzhe(char *name)
{
    init();
}

xingzhe::~xingzhe()
{
    if(left_foot != NULL)
        delete left_foot;
    if(right_foot != NULL)
        delete right_foot;
}

void xingzhe::init()
{
    m_state = INIT_STATE;
    left_foot = new XZ_Foot(LEFT_SIDE);
    right_foot = new XZ_Foot(RIGHT_SIDE);

    fd_mc = -1;
    fd_uc = -1;
    poolfd_num = 0;

    get_my_addr();
}

void xingzhe::get_my_addr()
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
        return;

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    s_my_ipaddr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    d_my_ipaddr = inet_network(s_my_ipaddr);

    close(fd);
}

int xingzhe::setup_mc_socket()
{
    struct sockaddr_in remote_addr, local_addr;
    
    char snd_buf[BUFLEN];
    u_int32 socklen, n;
    struct s_com *rcv_msg, *snd_msg;
    struct s_request_base *snd_payload;

    if ((fd_mc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        printf("socket creating error\n");
        return -1;
    }
    socklen = sizeof(struct sockaddr_in);

    memset(&remote_addr, 0, socklen);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(XZ_PORT2);

    if (inet_pton(AF_INET, MUL_IPADDR, &remote_addr.sin_addr) <= 0) 
    {
        printf("wrong group address!\n");
        return -2;
    }
    
    memset(&local_addr, 0, socklen);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(23456);
    
    if (inet_pton(AF_INET, s_my_ipaddr, &local_addr.sin_addr) <= 0) 
    {
        printf("self ip address error!\n");
        return -3;
    }

    if (bind(fd_mc, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) == -1) 
    {
        printf("Bind error\n");
        return -4;
    }
    
    bzero(snd_buf, BUFLEN);
    rcv_msg = (struct s_com *)snd_buf;
    rcv_msg->code = XZ_REQUEST_BASE;
    snd_payload = (struct s_request_base *)(snd_buf + sizeof(struct s_com));
    memcpy(snd_payload->name, MY_NAME, sizeof(MY_NAME));

    if (sendto(fd_mc, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in)) < 0) 
    {
        perror("sendto error");
        return -5;
    }
    m_state = WAITING_STATE;
    fdset[MC_MESSAGE].fd = fd_mc;
    fdset[MC_MESSAGE].events = POLLIN;
    msg_handler[MC_MESSAGE] = &xingzhe::deal_mc_msg;
    poolfd_num++;

    return 0;
}

int xingzhe::deal_mc_msg(void *data)
{
    return 0;
}

int xingzhe::setup_regular_report()
{
    return 0;
}

void xingzhe::proc()
{
    for(;;)
    {
        if(poolfd_num != 0) {
            if(poll(fdset, poolfd_num, -1)==0) {
                usleep(100);
                continue;
            }
            if(fdset[0].revents & POLLIN) {
                //((xingzhe*)this)->xingzhe::msg_handler[0](NULL);
            }
        }
        usleep(10);
    }
}

int xingzhe::wait_for_base()
{
    struct sockaddr_in remote_addr, local_addr;
    
    int sockfd;
    char rcv_buf[BUFLEN];
    char snd_buf[BUFLEN];
    u_int32 socklen, n;
    struct s_com *rcv_msg, *snd_msg;
    struct s_request_base *snd_payload;
    struct s_mc_ipaddr    *rcv_payload;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        printf("socket creating error\n");
        sleep(10);
    }
    socklen = sizeof(struct sockaddr_in);

    memset(&remote_addr, 0, socklen);
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(XZ_PORT2);

    while (inet_pton(AF_INET, MUL_IPADDR, &remote_addr.sin_addr) <= 0) 
    {
        printf("wrong group address!\n");
        sleep(10);
    }
    
    memset(&local_addr, 0, socklen);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(23456);
    
    while (inet_pton(AF_INET, s_my_ipaddr, &local_addr.sin_addr) <= 0) 
    {
        printf("self ip address error!\n");
        sleep(10);
    }

    while (bind(sockfd, (struct sockaddr *) &local_addr, sizeof(struct sockaddr_in)) == -1) 
    {
        printf("Bind error\n");
        sleep(10);
    }
    
    bzero(snd_buf, BUFLEN);
    rcv_msg = (struct s_com *)snd_buf;
    rcv_msg->code = XZ_REQUEST_BASE;
    snd_payload = (struct s_request_base *)(snd_buf + sizeof(struct s_com));
    memcpy(snd_payload->name, MY_NAME, sizeof(MY_NAME));

    while (sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in)) < 0) 
    {
        perror("sendto error");
        sleep(10);
    }

    for (;;) 
    {
        bzero(rcv_buf, BUFLEN);
        n = recvfrom(sockfd, rcv_buf, BUFLEN, MSG_DONTWAIT, (struct sockaddr *) &remote_addr, &socklen);
        while (n < sizeof(struct s_com)) {
            /* keep requesting unless received response from BASE */
            sendto(sockfd, snd_buf, sizeof(snd_buf), 0, (struct sockaddr *) &remote_addr, sizeof(struct sockaddr_in));
            sleep(1);
	        n = recvfrom(sockfd, rcv_buf, BUFLEN, MSG_DONTWAIT, (struct sockaddr *) &remote_addr, &socklen);
            continue;
        }
        rcv_msg = (struct s_com *)rcv_buf;
        switch(rcv_msg->code)
        {
            case BA_MC_IPADDR:
            {
                rcv_payload = (struct s_mc_ipaddr *)(rcv_msg + sizeof(struct s_com));
                d_base_ipaddr =  htonl(rcv_payload->ipaddr);
                close(sockfd);
                return 0;
            }
            default:
                break;
        }
    }

    return 0;
}

void *commu_proc(void *data)
{
    struct s_base_info m_base;
    char rcv_buf[BUFLEN];
    struct hostent *server_host_name;
    int n;
    struct s_com *msg;
    printf("communication thread started!\n");
    u_int32 baseip= *(u_int32 *)data;

    m_base.fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(m_base.fd < 0)
    {
        printf("Socket failed\n");
        return NULL;
    }
    printf("Socket SUCC!\n");
    
    memset(&m_base.m_addr, 0, sizeof(m_base.m_addr));
    m_base.m_addr.sin_family = AF_INET;
    m_base.m_addr.sin_addr.s_addr = htonl(baseip);
    m_base.m_addr.sin_port = htons(XZ_PORT1);
    
    while(connect(m_base.fd, (struct sockaddr *)&m_base.m_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect timeout!\n");
        sleep(10);
    }

    for(;;)
    {    
        n = recv(m_base.fd, (void *)rcv_buf, BUFLEN, MSG_WAITALL);
        if (n < sizeof(struct s_com)) {
            sleep(10);
            continue;
        }
        msg = (struct s_com *)rcv_buf;

        if(msg->code > 63)
        {
            printf("UNKNOWN command(%d) is received!\n", msg->code);
            continue;
        }
/*        if(cmd_cb[msg->code] == NULL)
        {
            printf("UNDEFINED command(%d) is received!\n", msg->code);
            continue;
        }
        
        cmd_cb[msg->code](rcv_buf + sizeof(struct s_com));
            case BA_LIGHT_CMD:
            {
                //printf("received  BA_LIGHT_CMD command from base\n");
                struct s_base_light *light_cmd = (struct s_base_light *)(rcv_buf + sizeof(struct s_com));
                if(light_cmd->on_off == BA_LIGHT_ON) {
                    set_led_on();
                }
                else {
                    set_led_off();
                }
                break;
            }

            case BA_CAMERA_CMD:
            {
                struct s_base_camera_cmd *camera_cmd = (struct s_base_camera_cmd *)(rcv_buf + sizeof(struct s_com));
                if(camera_cmd->on_off == 1) {
                    set_camera_rotate(1);
                }
                else {
                    set_camera_rotate(1);
                }
                break;
            }
            case BA_GC_SETTINGS:
            {
                //printf("received  BA_GC_SETTINGS command from base\n");
                struct s_base_light *light_cmd = (struct s_base_light *)(rcv_buf + sizeof(struct s_com));
                if(light_cmd->on_off == BA_LIGHT_ON) {
                    set_led_on();
                }
                else {
                    set_led_off();
                }
                break;
            }
            case BA_TEST_CMD:
            {
                parser_test_cmd();
                break;
            }
            case BA_SERVO_CMD:
            {
                struct s_base_servo_cmd *servo_cmd = (struct s_base_servo_cmd *)(rcv_buf + sizeof(struct s_com));

                if(servo_cmd->cmd == 1)  // 正转
                    servo_rotate_to(servo_cmd->id, servo_cmd->angle);
                else
                    servo_rotate_to(servo_cmd->id, -1*servo_cmd->angle);

                break;
            }
*/
    }
    
    close(m_base.fd);

    return NULL;
}

int xingzhe::deal_tm_msg(void *data)
{
    return 0;
}

int xingzhe::create_timer(int int_msec)
{
    struct itimerspec value;
    int fd;

    value.it_value.tv_sec = int_msec/1000;
    value.it_value.tv_nsec = int_msec%1000;
    value.it_interval.tv_sec = int_msec/1000;
    value.it_interval.tv_nsec = int_msec%1000;
    
    fd = timerfd_create(CLOCK_MONOTONIC, 0);
    timerfd_settime(fd, 0, &value, NULL);
    fdset[TM_MESSAGE_0].fd = fd;
    fdset[TM_MESSAGE_0].events = POLLIN;
    msg_handler[TM_MESSAGE_0] = &xingzhe::deal_tm_msg;
    poolfd_num++;
    
    return fd;
}

