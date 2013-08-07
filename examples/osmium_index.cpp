
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
class IndexSearch {

    typedef typename osmium::index::map::MmapFile<TKey, TValue> array_index_type;
    typedef typename osmium::index::map::MmapList<TKey, TValue> list_index_type;

    int m_fd;
    bool m_array_format;

    void dump_array() {
        array_index_type index(m_fd);

        for (size_t i = 0; i < index.size(); ++i) {
            if (index.get(i) != TValue()) {
                std::cout << i << " " << index.get(i) << "\n";
            }
        }
    }

    void dump_list() {
        list_index_type index(m_fd);

        for (auto& element : index) {
            std::cout << element.key << " " << element.value << "\n";
        }
    }

    bool search_array(TKey key) {
        array_index_type index(m_fd);

        try {
            TValue value = index.get(key);
            std::cout << key << " " << value << std::endl;
        } catch (...) {
            std::cout << key << " not found" << std::endl;
            return false;
        }

        return true;
    }

    bool search_list(TKey key) {
        typedef typename list_index_type::element_type element_type;
        list_index_type index(m_fd);

        element_type elem {key, TValue()};
        auto positions = std::equal_range(index.begin(), index.end(), elem, [](const element_type& lhs, const element_type& rhs) {
            return lhs.key < rhs.key;
        });
        if (positions.first == positions.second) {
            std::cout << key << " not found" << std::endl;
            return false;
        }

        for (auto& it = positions.first; it != positions.second; ++it) {
            std::cout << it->key << " " << it->value << "\n";
        }

        return true;
    }

public:

    IndexSearch(int fd, bool array_format) :
        m_fd(fd),
        m_array_format(array_format) {
    }

    void dump() {
        if (m_array_format) {
            dump_array();
        } else {
            dump_list();
        }
    }

    bool search(TKey key) {
        if (m_array_format) {
            return search_array(key);
        } else {
            return search_list(key);
        }
    }

    bool search(std::vector<TKey> keys) {
        bool found_all = true;

        for (const auto key : keys) {
            if (!search(key)) {
                found_all = false;
            }
        }

        return found_all;
    }

}; // class IndexSearch

enum return_code : int {
    okay      = 0,
    not_found = 1,
    error     = 2,
    fatal     = 3
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
            exit(return_code::okay);
        }

        if (vm.count("array") && vm.count("list")) {
            std::cerr << "Only option --array or --list allowed." << std::endl;
            exit(return_code::fatal);
        }

        if (!vm.count("array") && !vm.count("list")) {
            std::cerr << "Need one of option --array or --list." << std::endl;
            exit(return_code::fatal);
        }

        if (!vm.count("type")) {
            std::cerr << "Need --type argument." << std::endl;
            exit(return_code::fatal);
        }

        const std::string& type = vm["type"].as<std::string>();
        if (type != "location" && type != "offset") {
            std::cerr << "Unknown type '" << type << "'. Must be 'location' or 'offset'." << std::endl;
            exit(return_code::fatal);
        }

        return vm;
    } catch (boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        exit(return_code::fatal);
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

    bool okay = true;

    if (vm["type"].as<std::string>() == "location") {
        IndexSearch<osmium::object_id_type, osmium::Location> is(fd, array_format);

        if (vm.count("dump")) {
            is.dump();
        }

        if (vm.count("search")) {
            okay = is.search(vm["search"].as<std::vector<osmium::object_id_type>>());
        }
    } else {
        IndexSearch<osmium::object_id_type, size_t> is(fd, array_format);

        if (vm.count("dump")) {
            is.dump();
        }

        if (vm.count("search")) {
            okay = is.search(vm["search"].as<std::vector<osmium::object_id_type>>());
        }
    }

    exit(okay ? return_code::okay : return_code::not_found);
}

