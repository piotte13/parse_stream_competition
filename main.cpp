#include <iostream>
#include <chrono>
#include "libraries/simdjson/simdjson.h"
#include "libraries/simdjson/simdjson.cpp"


#include "libraries/rapidjson/document.h"
#include "libraries/rapidjson/writer.h"
#include "libraries/rapidjson/stringbuffer.h"
#include "libraries/rapidjson/error/en.h"

#include "libraries/mmapping/MemoryMapped.h"
#include "libraries/mmapping/MemoryMapped.cpp"

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define END_COLOR "\033[0m"

int nb_iterations = 10;
bool allow_mmap = true;

int main(int argc, char *argv[]) {
    if(argc < 2) {
        std::cerr << "Please specify a filename " << std::endl;
    }
    const char * filename = argv[1];
    const char* raw;
    size_t len;
    std::string data_string;
    simdjson::padded_string p;
    auto data = MemoryMapped{};
    if(allow_mmap){
        data.open(filename, MemoryMapped::WholeFile);
        raw = (const char*) data.getData();
        len = data.size();
        data_string = std::string(raw,len);
    }
    else{
        auto p1 = simdjson::get_corpus(filename);
        p.swap(p1);
        raw = p.data();
        len = p.size();
        data_string = std::string(raw,len);
    };



    std::cout << GREEN  << "simdjson: Getline + normal parse... " << END_COLOR << std::endl;
    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
    for (auto i = 0; i < nb_iterations; i++) {
        //Actual test
        simdjson::ParsedJson pj;
        bool allocok = pj.allocate_capacity(len);
        if (!allocok) {
            std::cerr << "failed to allocate memory" << std::endl;
            return EXIT_FAILURE;
        }
        std::istringstream ss(data_string);

        auto start = std::chrono::steady_clock::now();
        int count = 0;
        std::string line;
        int parse_res = simdjson::SUCCESS;
        while (getline(ss, line) && parse_res == simdjson::SUCCESS) {
            parse_res = simdjson::json_parse(line, pj);
            count++;
        }

        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> secs = end - start;
        double speedinGBs = (len) / (secs.count() * 1000000000.0);
        std::cout << std::setprecision(3) << speedinGBs << "\t\t\t\t" << count << std::endl;

        if (parse_res != simdjson::SUCCESS) {
            std::wcerr << "Parsing failed with: " << simdjson::error_message(parse_res).c_str() << std::endl;
            exit(1);
        }
    }



//    std::cout << MAGENTA << "rapidjson: Getline + normal parse... " << END_COLOR << std::endl;
//    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
//    for (auto i = 0; i < nb_iterations; i++) {
//        //Actual test
//        std::istringstream ss(data_string);
//        rapidjson::Document d;
//
//        auto start = std::chrono::steady_clock::now();
//        int count = 0;
//        std::string line;
//        while (getline(ss, line) && !d.HasParseError()) {
//            d.Parse(line.data());
//            count++;
//        }
//
//        auto end = std::chrono::steady_clock::now();
//        d.Clear();
//        std::chrono::duration<double> secs = end - start;
//        double speedinGBs = (len) / (secs.count() * 1000000000.0);
//        std::cout << std::setprecision(3) << speedinGBs << "\t\t\t\t" << count << std::endl;
//
//        if (d.HasParseError()) {
//            std::wcerr << "Parsing failed with: " <<  GetParseError_En(d.GetParseError()) << std::endl;
//            exit(1);
//        }
//    }




    std::cout << BLUE << "simdjson: jsonstream" << END_COLOR << std::endl;
    std::cout << "Gigabytes/second\t" << "Nb of documents parsed" << std::endl;
    for (auto i = 0; i < nb_iterations; i++) {
        //Actual test
        simdjson::ParsedJson pj;
        simdjson::JsonStream js{raw, len, 300000};
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
            std::wcerr << "Parsing failed with: " << simdjson::error_message(parse_res).c_str() << std::endl;
            exit(1);
        }
    }

    return EXIT_SUCCESS;
}