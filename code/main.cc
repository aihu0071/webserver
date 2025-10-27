#include "server/web_server.h"

int main(){
    WebServer server(8080, 3, 60000, 3306, 
                    "aihu", "password", "web_server",
                    12, 8, true, 1, 1024);
    server.start();
    return 0;
}