#include <iostream>
#include <chrono>
#include "libraries/simdjson/simdjson.h"
#include "libraries/simdjson/simdjson.cpp"


#include "libraries/rapidjson/document.h"
#include "libraries/rapidjson/writer.h"
#include "libraries/rapidjson/stringbuffer.h"

#include "libraries/mmapping/MemoryMapped.h"
#include "libraries/mmapping/MemoryMapped.cpp"

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please specify a filename " << std::endl;
    }
    const char * filename = argv[1];
    //simdjson::padded_string p = simdjson::get_corpus(filename);
    MemoryMapped data(filename, MemoryMapped::WholeFile);
    const char* raw = (const char*) data.getData();
    const size_t len = data.size();

    std::wclog << "simdjson: Getline + normal parse... " << std::endl;
    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
    for (auto i = 0; i < 1; i++) {
        //Actual test
        simdjson::ParsedJson pj;
        bool allocok = pj.allocate_capacity(len);
        if (!allocok) {
            std::cerr << "failed to allocate memory" << std::endl;
            return EXIT_FAILURE;
        }
        std::istringstream ss(std::string(raw,len));

        auto start = std::chrono::steady_clock::now();
        int count = 0;
        std::string line;
        int parse_res = simdjson::SUCCESS;
        while (getline(ss, line)) {
            parse_res = simdjson::json_parse(line, pj);
            count++;
        }

        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> secs = end - start;
        double speedinGBs = (len) / (secs.count() * 1000000000.0);
        std::cout << std::setprecision(3) << speedinGBs << "\t\t\t\t" << count << std::endl;

        if (parse_res != simdjson::SUCCESS) {
            std::cerr << "Parsing failed" << std::endl;
            exit(1);
        }
    }



    std::wclog << "rapidjson: Getline + normal parse... " << std::endl;
    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
    for (auto i = 0; i < 1; i++) {
        //Actual test
        std::istringstream ss(std::string(raw, len));
        rapidjson::Document d;

        auto start = std::chrono::steady_clock::now();
        int count = 0;
        std::string line;
        int parse_res = simdjson::SUCCESS;
        while (getline(ss, line)) {
            d.Parse(line.data());
            count++;
        }

        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> secs = end - start;
        double speedinGBs = (len) / (secs.count() * 1000000000.0);
        std::cout << std::setprecision(3) << speedinGBs << "\t\t\t\t" << count << std::endl;
    }




    std::wclog << "simdjson: jsonstream \n" << std::endl;
    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
    for (auto i = 0; i < 1; i++) {
        //Actual test
        simdjson::ParsedJson pj;
        simdjson::JsonStream js{raw, len, 1000000};
        int parse_res = simdjson::SUCCESS_AND_HAS_MORE;

        auto start = std::chrono::steady_clock::now();

        int count = 0;
        while (parse_res == simdjson::SUCCESS_AND_HAS_MORE) {
            parse_res = js.json_parse(pj);
            count++;
        }


        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> secs = end - start;
        double speedinGBs = (len) / (secs.count() * 1000000000.0);
        std::cout << std::setprecision(3) << speedinGBs << "\t\t\t\t" << count << std::endl;

        if (parse_res != simdjson::SUCCESS) {
            std::cerr << "Parsing failed" << std::endl;
            exit(1);
        }
    }

    return EXIT_SUCCESS;
}