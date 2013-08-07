
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
#include <osmium/index/map/mmap_list.hpp>
#include <osmium/index/map/mmap_file.hpp>

template <typename TKey, typename TValue>
void dump_array(int fd) {
    osmium::index::map::MmapFile<TKey, TValue> index(fd);

    for (size_t i = 0; i < index.size(); ++i) {
        if (index.get(i) != TValue()) {
            std::cout << i << " " << index.get(i) << "\n";
        }
    }
}

template <typename TKey, typename TValue>
void dump_list(int fd) {
    osmium::index::map::MmapList<TKey, TValue> index(fd);

    for (auto& element : index) {
        std::cout << element.key << " " << element.value << "\n";
    }
}

template <typename TKey, typename TValue>
bool search_array(int fd, TKey id) {
    typedef typename osmium::index::map::MmapFile<TKey, TValue> index_type;
    index_type index(fd);

    try {
        TValue found = index.get(id);
        std::cout << id << " " << found << std::endl;
    } catch (...) {
        return false;
    }

    return true;
}

template <typename TKey, typename TValue>
bool search_list(int fd, TKey id) {
    typedef typename osmium::index::map::MmapList<TKey, TValue> index_type;
    index_type index(fd);

    typename index_type::element_type elem {id, TValue()};
    auto positions = std::equal_range(index.begin(), index.end(), elem, [](const typename index_type::element_type& lhs, const typename index_type::element_type& rhs) {
        return lhs.key < rhs.key;
    });
    if (positions.first == positions.second) {
        return false;
    }

    for (auto& it = positions.first; it != positions.second; ++it) {
        std::cout << it->key << " " << it->value << "\n";
    }

    return true;
}

enum return_type : int {
    okay = 0,
    not_found = 1,
    error = 2,
    fatal = 3
};

boost::program_options::variables_map parse_options(int argc, char* argv[]) {
    namespace po = boost::program_options;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "Print this help message")
            ("array,a", po::value<std::string>(), "Read given index file in array format")
            ("list,l", po::value<std::string>(), "Read given index file in list format")
            ("dump,d", "Dump contents of index file to STDOUT")
            ("search,s", po::value<std::vector<osmium::object_id_type>>(), "Search for given id (Option can appear multiple times)")
            ("type,t", po::value<std::string>(), "Type of value ('location' or 'offset')")
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

        if (!vm.count("type")) {
            std::cerr << "Need --type argument." << std::endl;
            exit(return_type::fatal);
        }

        const std::string& type = vm["type"].as<std::string>();
        if (type != "location" && type != "offset") {
            std::cerr << "Unknown type '" << type << "'. Must be 'location' or 'offset'." << std::endl;
            exit(return_type::fatal);
        }

        return vm;
    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        exit(return_type::fatal);
    }
}

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    auto vm = parse_options(argc, argv);

    std::string filename;
    bool array_format = false;

    if (vm.count("array")) {
        filename = vm["array"].as<std::string>();
        array_format = true;
    }
    if (vm.count("list")) {
        filename = vm["list"].as<std::string>();
    }

    std::cout << std::fixed << std::setprecision(7);
    int fd = open(filename.c_str(), O_RDONLY);

    if (vm.count("dump")) {
        if (array_format) {
            if (vm["type"].as<std::string>() == "location") {
                dump_array<osmium::object_id_type, osmium::Location>(fd);
            } else {
                dump_array<osmium::object_id_type, size_t>(fd);
            }
        } else {
            if (vm["type"].as<std::string>() == "location") {
                dump_list<osmium::object_id_type, osmium::Location>(fd);
            } else {
                dump_list<osmium::object_id_type, size_t>(fd);
            }
        }
    }

    if (vm.count("search")) {
        std::vector<osmium::object_id_type> ids = vm["search"].as<std::vector<osmium::object_id_type>>();

        bool found = true;
        for (const auto id : ids) {
            bool okay = array_format ? (vm["type"].as<std::string>() == "location" ? search_array<osmium::object_id_type, osmium::Location>(fd, id)
                                                                                   : search_array<osmium::object_id_type, size_t>(fd, id))
                                     : (vm["type"].as<std::string>() == "location" ? search_list<osmium::object_id_type, osmium::Location>(fd, id)
                                                                                   : search_list<osmium::object_id_type, size_t>(fd, id));
            if (!okay) {
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

