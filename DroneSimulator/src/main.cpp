#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    std::cout << "Drone Simulator starting..." << std::endl;

    while (true)
    {
        std::cout << "Drone Simulator is running..." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}