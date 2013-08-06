
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/program_options.hpp>

#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/ostream.hpp>

namespace po = boost::program_options;

template <typename T>
void dump(void* ptr, size_t size) {
    T* data = reinterpret_cast<T*>(ptr);
    size_t count = size / sizeof(T);

    std::cout << std::fixed << std::setprecision(7);
    for (size_t i = 0; i < count; ++i) {
        std::cout << data[i].first << " " << data[i].second << "\n";
    }
}

template <typename T>
bool search(void* ptr, size_t size, osmium::object_id_type id) {
    T* data = reinterpret_cast<T*>(ptr);
    size_t count = size / sizeof(T);

    T elem {id, osmium::Location()};
    auto positions = std::equal_range(data, data+count, elem, [](const T& lhs, const T& rhs) {
        return lhs.first < rhs.first;
    });
    if (positions.first == positions.second) {
        return false;
    }

    std::cout << std::fixed << std::setprecision(7);

    for (auto& it = positions.first; it != positions.second; ++it) {
        std::cout << it->first << " " << it->second << "\n";
    }

    return true;
}

enum return_type : int {
    okay = 0,
    not_found = 1,
    error = 2,
    fatal = 3
};

po::variables_map parse_options(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Print this help message")
            ("array,a", po::value<std::string>(), "Read given index file in array format")
            ("list,l", po::value<std::string>(), "Read given index file in list format")
            ("dump,d", "Dump contents of index file to STDOUT")
            ("search,s", po::value<std::vector<osmium::object_id_type>>(), "Search for given id (Option can appear multiple times)")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);    

        if (vm.count("help")) {
            std::cout << desc << "\n";
            exit(return_type::okay);
        }

        if (vm.count("array") && vm.count("list")) {
            std::cerr << "Only option --array or --list allowed." << std::endl;
            exit(return_type::fatal);
        }

        if (!vm.count("array") && !vm.count("list")) {
            std::cerr << "Need one of option --array or --list." << std::endl;
            exit(return_type::fatal);
        }
        return vm;
    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        exit(return_type::fatal);
    }
}

typedef std::pair<osmium::object_id_type, osmium::Location> id2loc_type;
typedef std::pair<osmium::object_id_type, size_t> id2offset_type;

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    po::variables_map vm = parse_options(argc, argv);

    std::string filename;
    bool array_format = false;
    
    if (vm.count("array")) {
        filename = vm["array"].as<std::string>();
        array_format = true;
    }
    if (vm.count("list")) {
        filename = vm["list"].as<std::string>();
    }

    int fd = open(filename.c_str(), O_RDONLY);
    struct stat file_stat;
    if (::fstat(fd, &file_stat) < 0) {
        std::cerr << "Can not stat index file" << std::endl;
        exit(return_type::error);
    }

    void* ptr = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Mmap failed" << std::endl;
        exit(return_type::error);
    }

    if (vm.count("dump")) {
        dump<id2loc_type>(ptr, file_stat.st_size);
    }

    if (vm.count("search")) {
        std::vector<osmium::object_id_type> ids = vm["search"].as<std::vector<osmium::object_id_type>>();

        bool found = true;
        for (const auto id : ids) {
            if (!search<id2loc_type>(ptr, file_stat.st_size, id)) {
                found = false;
                std::cout << id << " not found\n";
            }
        }

        if (found) {
            exit(return_type::okay);
        } else {
            exit(return_type::not_found);
        }
    }
}

