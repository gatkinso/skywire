#include <iostream>
#include <string>

#include "Point.h"

int main()
{
    demo::Point p;
    p.Set_x(0x1234);
    p.Set_y(0x5678);

    const std::string json = p.to_json();
    std::cout << json << std::endl;

    demo::Point back;
    return json == R"({"x":4660,"y":22136})" && back.from_json(json) && back == p ? 0 : 1;
}
