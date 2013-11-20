#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <iterator>

#include <osmium/io/file.hpp>

BOOST_AUTO_TEST_SUITE(FileFormats)

BOOST_AUTO_TEST_CASE(default_file_format) {
    {
        osmium::io::File file;
        BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, file.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, file.compression());
        BOOST_CHECK_EQUAL(false, file.has_multiple_object_versions());
    }

    {
        osmium::io::File file {""};
        BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, file.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, file.compression());
        BOOST_CHECK_EQUAL(false, file.has_multiple_object_versions());
    }

    {
        osmium::io::File file {"-"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, file.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, file.compression());
        BOOST_CHECK_EQUAL(false, file.has_multiple_object_versions());
    }
}

BOOST_AUTO_TEST_CASE(detect_file_format_by_suffix) {
    {
        osmium::io::File f {"test.osm"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osm.pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.opl"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osm.opl"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osm.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osm.opl.bz2"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::bzip2, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osc.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osh.opl.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test.osh.pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }

}

BOOST_AUTO_TEST_CASE(override_file_format_by_suffix) {
    {
        osmium::io::File f {"test", "osm"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osm.pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "opl"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osm.opl"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osm.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osm.opl.bz2"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::bzip2, f.compression());
        BOOST_CHECK_EQUAL(false, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osc.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osh.opl.gz"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::opl, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::gzip, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }

    {
        osmium::io::File f {"test", "osh.pbf"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }
}

BOOST_AUTO_TEST_CASE(format_options) {
    {
        osmium::io::File f {"test", "pbf,history=true"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL(true, f.has_multiple_object_versions());
    }
    {
        osmium::io::File f {"test.osm", "pbf,foo=bar"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::pbf, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL("bar", f.get("foo"));
    }
    {
        osmium::io::File f {"test.bla", "xml,xml_josm_upload,some=thing"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::xml, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
        BOOST_CHECK_EQUAL("true", f.get("xml_josm_upload"));
        BOOST_CHECK_EQUAL("thing", f.get("some"));
        BOOST_CHECK_EQUAL(2, std::distance(f.begin(), f.end()));
    }
}

BOOST_AUTO_TEST_CASE(unknown_format) {
    {
        osmium::io::File f {"test.foo.bar"};
        BOOST_CHECK_EQUAL(osmium::io::file_format::unknown, f.format());
        BOOST_CHECK_EQUAL(osmium::io::file_compression::none, f.compression());
    }
    {
        BOOST_CHECK_THROW(osmium::io::File("test", "foo"), std::runtime_error);
    }
    {
        BOOST_CHECK_THROW(osmium::io::File("test", "osm.foo"), std::runtime_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()

