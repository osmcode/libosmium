#include "test_utils.hpp"

namespace Osmium {

    namespace Test {

        std::string to_hex(const std::string& in) {
            static const char* lookup_hex = "0123456789ABCDEF";
            std::string out("");
            for (std::string::const_iterator it=in.begin(); it != in.end(); ++it) {
                out += lookup_hex[(*it >> 4) & 0xf];
                out += lookup_hex[*it & 0xf];
            }
            return out;
        }

    } // namespace Test

} // namespace Osmium
