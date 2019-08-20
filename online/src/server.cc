#include "WordQueryServer.h"

int main()
{
    wd::WordQueryServer server("../conf/my.conf");
    server.start();
    return 0;
}

