#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

#include <iostream>

#include "pub.h"
#include "common.h"

#include "xingzhe.h"

using namespace std;

static void tm_handler(int id)
{
    printf("id = %d\n", id);
}

int main(int argc, char *argv[])
{
    xingzhe *wukong;
    
    cout << "Hello!" << endl;

    wukong = new xingzhe("dasheng");
    
    wukong->init();
    wukong->proc();

    delete wukong;

    return 0;
}


