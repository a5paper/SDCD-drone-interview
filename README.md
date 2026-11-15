# SDCD-for-interview

## How to compile
1. Use a Ubuntu 24 LTS or a windows computer the same OS on WSL.
2. `sudo bash install-deps.sh`
3. `sudo bash compile.sh`

## Assumptions
For the sake of the first Task I made several assumptions:
1. Simple physics — I assume the drone moves at either a constant speed or 0, since I done want to deal with acceloration.
2. World Simulation — For the sake of this simulation I am making it all in the same project and creating an in code seperation between the drone simulation and the world simulation.
3. The world doesn't particularly matter — Since I want to simulate a drone, and not a lively world, I am going to put minimal effort into the camera, much like the question asks.
4. Build environemnt — This project is built on WSL ubuntu 24.