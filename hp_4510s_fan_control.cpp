/*
 * Copyright 2016 Dominik Wójt
 *
 * This file contains code for controling fan speed of HP 4510s laptop computer.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

const char help[] =
R"(This software is based on https://github.com/strelec/HP-4510s-fan-control

Modifications to the original are:
- fixed cooling_device numbers, this is probably due to BIOS update in the
  laptop I poses.
- changed temperature measurement to lm-sensors, the values read from
  /sys/class/hwmon/hwmon0/temp were far from those from lm-sensors
- language changed to C++. I don't know Ruby ;))

Usage:
    hp_4510s_fan_control CONFIG_FILE

CONFIG_FILE
    config file should contain 10 numbers separated with any whitespace.
    The numbers are T1_on, T1_off, T2_on, T2_off..., T5_off, T5_on
    T?_off is temperature at which given speed level is lowered,
    T?_on is temperature at which given speed level is increased
)";

#include <vector>
#include <numeric>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <thread>

const int history_lenght = 5;
const std::chrono::milliseconds update_period(1000);
const int speed_levels_count = 6; /* i.e. 0..5 */
const char *cooling_devices[] =
{
    "/sys/devices/virtual/thermal/cooling_device0/cur_state",
    "/sys/devices/virtual/thermal/cooling_device1/cur_state",
    "/sys/devices/virtual/thermal/cooling_device2/cur_state",
    "/sys/devices/virtual/thermal/cooling_device3/cur_state",
    "/sys/devices/virtual/thermal/cooling_device4/cur_state"
};

const char *monitor_devices[] =
{
    "/sys/class/hwmon/hwmon0/temp3_input",
    "/sys/class/hwmon/hwmon0/temp4_input",
};

template<typename TValue>
class History
{
public:
    using Value = TValue;
private:
    std::vector<Value> m_buffer;
    int m_index;
    int m_lenght;
public:
    History(int lenght) :
        m_buffer(),
        m_index(0),
        m_lenght(lenght)
    {
        m_buffer.reserve(lenght);
    }

    void update(Value x)
    {
        if(m_buffer.size() < m_lenght)
            m_buffer.push_back(x);
        else
            m_buffer[m_index] = x;
        m_index = (m_index + 1) % m_lenght;
    }

    Value average() const
    {
        if(m_buffer.empty())
            return 0;
        else
            return
                    std::accumulate(
                        m_buffer.begin(),
                        m_buffer.end(),
                        static_cast<double>(0))
                    / m_buffer.size();
    }
};

double read_temperature()
{
    if(true)
    {
        double z;
        std::cin >> z;
        if(!std::cin.good())
            throw std::runtime_error("failed to read input");
        return z;
    }
    double max = 0;
    for(const auto monitor_device : monitor_devices)
    {
        std::ifstream file(monitor_device);
        double z;
        file >> z;
        z /= 1000;
        if(z > max)
            max = z;
    }
    return max;
}

void set_speed_level(int speed_level)
{
    if(true)
    {
        std::cout << "speed level: " << speed_level << '\n';
        return;
    }
    const int cooling_devices_count =
            sizeof(cooling_devices) / sizeof(cooling_devices[0]);
    for(int i = 0; i < cooling_devices_count; ++i)
    {
        std::ofstream file(cooling_devices[i]);
        if(i < speed_level)
        {
            file << 1;
        }
        else
        {
            file << 0;
        }
    }
}

struct Config
{
    struct Level_limits
    {
        double off;
        double on;
    };

    Level_limits levels[speed_levels_count];
};

[[ noreturn ]] void monitor(const Config &config)
{
    History<double> temperature_history(history_lenght);
    int speed_level = 0;

    while(true)
    {
        temperature_history.update(read_temperature());
        const double average = temperature_history.average();

        while(
                speed_level + 1 < speed_levels_count
                && average > config.levels[speed_level + 1].on)
        {
            ++speed_level;
        }
        while(
                speed_level - 1 >= 0
                && average < config.levels[speed_level].off)
        {
            --speed_level;
        }

        std::cout
                << "T = " << average
                << ", speed_level = " << speed_level << '\n';
        set_speed_level(speed_level);

        std::this_thread::sleep_for(update_period);
    }
}

Config read_config(const std::string &config_name)
{
    std::ifstream config_file(config_name);

    Config result;

    result.levels[0].off = 0;
    result.levels[0].on = 0.1;
    for(int i = 1; i < speed_levels_count; ++i)
    {
        config_file >> result.levels[i].off >> result.levels[i].on;
    }
    if(!config_file.good())
        throw std::runtime_error("failed to read config file");

    for(int i = 1; i < speed_levels_count; ++i)
    {
        if(result.levels[i - 1].off > result.levels[i].off)
            throw std::runtime_error(
                    "off temperatures are not non-decreasing with speed level");
        if(result.levels[i - 1].on > result.levels[i].on)
            throw std::runtime_error(
                    "on temperatures are not non-decreasing with speed level");
    }

    for(int i = 0; i < speed_levels_count; ++i)
        if(result.levels[i].on <= result.levels[i].off)
            throw std::runtime_error(
                    "on temperature is not greater than off temperature");

    return result;
}

int main(const int argc, const char *const argv[]) try
{
    if(argc != 2)
        throw std::runtime_error("invalid commandline");

    const Config config = read_config(argv[1]);

    while(true)
    {
        try
        {
            monitor(config);
        }
        catch(std::exception &e)
        {
            std::cerr
                    << "std::exception caught: " << e.what() << '\n'
                    << "restarting...\n";
        }
        catch(...)
        {
            std::cerr
                    << "unknown caught\n"
                    << "restarting...\n";
        }
        std::this_thread::sleep_for(update_period);
    }
}
catch(std::exception &e)
{
    std::cerr << "std::exception caught: " << e.what() << '\n';
    std::cerr << help;
    return -1;
}
catch(...)
{
    std::cerr << "unknown caught\n";
    std::cerr << help;
    return -1;
}
