#ifndef __XINGZHE_H__
#define __XINGZHE_H__

#include <poll.h>

#include "eye.h"
#include "ear.h"
#include "foot.h"

struct commu_param {
    struct s_base_info m_base;
    u_int32 d_base_ipaddr;
};

enum {
    INIT_STATE = 0,
    WAITING_STATE,
    WORKING_STATE,
};

class xingzhe {
public:
    xingzhe();
    xingzhe(char *name);
    ~xingzhe();

    void proc();

    typedef int(xingzhe::*XZ_MSG_HANDLER)(void *);
    XZ_MSG_HANDLER msg_handler[64];

private:
    void init();
    void get_my_addr();
    int wait_for_base();
    int setup_mc_socket();
    int setup_regular_report();
    int create_timer(int msec);
    int deal_mc_msg(void *data);
    int deal_tm_msg(void *data);

private:
    XZ_Eye *eye = NULL;  // camera
    XZ_Ear *ear = NULL;  // usonic sensor
    XZ_Foot *left_foot = NULL;  //wheel
    XZ_Foot *right_foot = NULL;  //wheel

    char *s_my_ipaddr;
    u_int32 d_my_ipaddr;
    u_int32 d_base_ipaddr;

    u_int32 m_state;
    int fd_mc;
    int fd_uc;
    int poolfd_num;

    enum POLL_EVENT {
        MC_MESSAGE = 0,
        UC_MESSAGE,
        TM_MESSAGE_0,
    };
    
//    head(); // servo;light
//    brain(); // decision
//    speed(); // speed
//    westbus(); // communication with base
    

protected:

public:
    
};


#endif

