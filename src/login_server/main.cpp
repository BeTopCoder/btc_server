#include <iostream>
#include "server.h"
#include <string>
using namespace std;

int main(int argc, char** argv)
{
    try
    {
        http::server::server server_(std::string("0.0.0.0"), std::string("10000"));
        server_.run();
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
    }
    getchar();
    return 0;
}