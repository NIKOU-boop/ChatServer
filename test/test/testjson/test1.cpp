#include <iostream>
#include <vector>
#include <map>
using namespace std;
#include "json.hpp"
#include <string>
using json = nlohmann::json;

void func1()
{
    json js;
    js["name"] = "John Doe";
    js["age"] = 30;
    js["is_student"] = false;
    js["courses"] = {"Math", "Science", "History"};
    string sendbuffer = js.dump(); // serialize to string
    cout << sendbuffer.c_str() << endl; // pretty print with indentation of 4 spaces
}

int main() {
    func1();
    return 0;
}