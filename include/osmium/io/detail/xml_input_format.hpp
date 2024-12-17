#ifndef OSMIUM_IO_DETAIL_XML_INPUT_FORMAT_HPP
#define OSMIUM_IO_DETAIL_XML_INPUT_FORMAT_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2024 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <osmium/builder/builder.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/detail/input_format.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/error.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/timestamp.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/osm/types_from_string.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/thread/util.hpp>

#include <expat.h>

#include <cassert>
#include <cstring>
#include <exception>
#include <future>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace osmium {

    /**
     * Exception thrown when the XML parser failed. The exception contains
     * (if available) information about the place where the error happened
     * and the type of error.
     */
    struct xml_error : public io_error {

        uint64_t line = 0;
        uint64_t column = 0;
        XML_Error error_code;
        std::string error_string;

        explicit xml_error(const XML_Parser& parser) :
            io_error(std::string{"XML parsing error at line "}
                    + std::to_string(XML_GetCurrentLineNumber(parser))
                    + ", column "
                    + std::to_string(XML_GetCurrentColumnNumber(parser))
                    + ": "
                    + XML_ErrorString(XML_GetErrorCode(parser))),
            line(XML_GetCurrentLineNumber(parser)),
            column(XML_GetCurrentColumnNumber(parser)),
            error_code(XML_GetErrorCode(parser)),
            error_string(XML_ErrorString(error_code)) {
        }

        explicit xml_error(const std::string& message) :
            io_error(message),
            error_code(),
            error_string(message) {
        }

    }; // struct xml_error

    /**
     * Exception thrown when an OSM XML files contains no version attribute
     * on the 'osm' element or if the version is unknown.
     */
    struct format_version_error : public io_error {

        std::string version;

        explicit format_version_error() :
            io_error("Can not read file without version (missing version attribute on osm element).") {
        }

        explicit format_version_error(const char* v) :
            io_error(std::string{"Can not read file with version "} + v),
            version(v) {
        }

    }; // struct format_version_error

    namespace io {

        namespace detail {

            class XMLParser final : public ParserWithBuffer {

                enum class context {
                    osm,
                    osmChange,
                    bounds,
                    create_section,
                    modify_section,
                    delete_section,
                    node,
                    way,
                    relation,
                    tag,
                    nd,
                    member,
                    changeset,
                    discussion,
                    comment,
                    text,
                    obj_bbox,
                    other
                }; // enum class context

                std::vector<context> m_context_stack;

                osmium::io::Header m_header;

                std::unique_ptr<osmium::builder::NodeBuilder>                m_node_builder;
                std::unique_ptr<osmium::builder::WayBuilder>                 m_way_builder;
                std::unique_ptr<osmium::builder::RelationBuilder>            m_relation_builder;
                std::unique_ptr<osmium::builder::ChangesetBuilder>           m_changeset_builder;
                std::unique_ptr<osmium::builder::ChangesetDiscussionBuilder> m_changeset_discussion_builder;

                std::unique_ptr<osmium::builder::TagListBuilder>             m_tl_builder;
                std::unique_ptr<osmium::builder::WayNodeListBuilder>         m_wnl_builder;
                std::unique_ptr<osmium::builder::RelationMemberListBuilder>  m_rml_builder;

                std::string m_comment_text;

                /**
                 * A C++ wrapper for the Expat parser that makes sure no memory
                 * is leaked.
                 */
                class ExpatXMLParser {

                    XML_Parser m_parser;
                    std::exception_ptr m_exception_ptr{}; // NOLINT(bugprone-throw-keyword-missing) see https://bugs.llvm.org/show_bug.cgi?id=52400

                    template <typename TFunc>
                    void member_wrap(XMLParser& xml_parser, TFunc&& func) noexcept {
                        if (m_exception_ptr) {
                            return;
                        }
                        try {
                            func(xml_parser);
                        } catch (...) {
                            m_exception_ptr = std::current_exception();
                            XML_StopParser(m_parser, 0);
                        }
                    }

                    template <typename TFunc>
                    static void wrap(void* data, TFunc&& func) noexcept {
                        assert(data);
                        auto& xml_parser = *static_cast<XMLParser*>(data);
                        xml_parser.m_expat_xml_parser->member_wrap(xml_parser, std::forward<TFunc>(func));
                    }

                    static void XMLCALL start_element_wrapper(void* data, const XML_Char* element, const XML_Char** attrs) noexcept {
                        wrap(data, [&](XMLParser& xml_parser) {
                            xml_parser.start_element(element, attrs);
                        });
                    }

                    static void XMLCALL end_element_wrapper(void* data, const XML_Char* element) noexcept {
                        wrap(data, [&](XMLParser& xml_parser) {
                            xml_parser.end_element(element);
                        });
                    }

                    static void XMLCALL character_data_wrapper(void* data, const XML_Char* text, int len) noexcept {
                        wrap(data, [&](XMLParser& xml_parser) {
                            xml_parser.characters(text, len);
                        });
                    }

                    // This handler is called when there are any XML entities
                    // declared in the OSM file. Entities are normally not used,
                    // but they can be misused. See
                    // https://en.wikipedia.org/wiki/Billion_laughs
                    // The handler will just throw an error.
                    static void entity_declaration_handler(void* data,
                            const XML_Char* /*entityName*/,
                            int /*is_parameter_entity*/,
                            const XML_Char* /*value*/,
                            int /*value_length*/,
                            const XML_Char* /*base*/,
                            const XML_Char* /*systemId*/,
                            const XML_Char* /*publicId*/,
                            const XML_Char* /*notationName*/) noexcept {
                        wrap(data, [&](XMLParser& /*xml_parser*/) {
                            throw osmium::xml_error{"XML entities are not supported"};
                        });
                    }

                public:

                    explicit ExpatXMLParser(void* callback_object) :
                        m_parser(XML_ParserCreate(nullptr)) {
                        if (!m_parser) {
                            throw osmium::io_error{"Internal error: Can not create parser"};
                        }
                        XML_SetUserData(m_parser, callback_object);
                        XML_SetElementHandler(m_parser, start_element_wrapper, end_element_wrapper);
                        XML_SetCharacterDataHandler(m_parser, character_data_wrapper);
                        XML_SetEntityDeclHandler(m_parser, entity_declaration_handler);
                    }

                    ExpatXMLParser(const ExpatXMLParser&) = delete;
                    ExpatXMLParser& operator=(const ExpatXMLParser&) = delete;

                    ExpatXMLParser(ExpatXMLParser&&) = delete;
                    ExpatXMLParser& operator=(ExpatXMLParser&&) = delete;

                    ~ExpatXMLParser() noexcept {
                        XML_ParserFree(m_parser);
                    }

                    void operator()(const std::string& data, bool last) {
                        assert(data.size() < std::numeric_limits<int>::max());
                        if (XML_Parse(m_parser, data.data(), static_cast<int>(data.size()), last) == XML_STATUS_ERROR) {
                            if (m_exception_ptr) {
                                std::rethrow_exception(m_exception_ptr);
                            }
                            throw osmium::xml_error{m_parser};
                        }
                    }

                }; // class ExpatXMLParser

                ExpatXMLParser* m_expat_xml_parser{nullptr};

                template <typename T>
                static void check_attributes(const XML_Char** attrs, T&& check) {
                    while (*attrs) {
                        check(attrs[0], attrs[1]);
                        attrs += 2;
                    }
                }

                const char* init_object(osmium::OSMObject& object, const XML_Char** attrs) {
                    assert(m_context_stack.size() > 1);
                    if (m_context_stack[m_context_stack.size() - 2] == context::delete_section) {
                        object.set_visible(false);
                    }

                    osmium::Location location;
                    const char* user = "";

                    check_attributes(attrs, [&location, &user, &object](const XML_Char* name, const XML_Char* value) {
                        if (!std::strcmp(name, "lon")) {
                            location.set_lon(value);
                        } else if (!std::strcmp(name, "lat")) {
                            location.set_lat(value);
                        } else if (!std::strcmp(name, "user")) {
                            user = value;
                        } else {
                            object.set_attribute(name, value);
                        }
                    });

                    if (location && object.type() == osmium::item_type::node) {
                        static_cast<osmium::Node&>(object).set_location(location);
                    }

                    return user;
                }

                static void init_changeset(osmium::builder::ChangesetBuilder& builder, const XML_Char** attrs) {
                    osmium::Box box;

                    check_attributes(attrs, [&builder, &box](const XML_Char* name, const XML_Char* value) {
                        if (!std::strcmp(name, "min_lon")) {
                            box.bottom_left().set_lon(value);
                        } else if (!std::strcmp(name, "min_lat")) {
                            box.bottom_left().set_lat(value);
                        } else if (!std::strcmp(name, "max_lon")) {
                            box.top_right().set_lon(value);
                        } else if (!std::strcmp(name, "max_lat")) {
                            box.top_right().set_lat(value);
                        } else if (!std::strcmp(name, "user")) {
                            builder.set_user(value);
                        } else {
                            builder.set_attribute(name, value);
                        }
                    });

                    builder.set_bounds(box);
                }

                void get_tag(osmium::builder::Builder& builder, const XML_Char** attrs) {
                    const char* k = "";
                    const char* v = "";

                    check_attributes(attrs, [&k, &v](const XML_Char* name, const XML_Char* value) {
                        if (name[0] == 'k' && name[1] == '\0') {
                            k = value;
                        } else if (name[0] == 'v' && name[1] == '\0') {
                            v = value;
                        }
                    });

                    if (!m_tl_builder) {
                        m_tl_builder = std::make_unique<osmium::builder::TagListBuilder>(builder);
                    }
                    m_tl_builder->add_tag(k, v);
                }

                void mark_header_as_done() {
                    set_header_value(m_header);
                }

                void top_level_element(const XML_Char* element, const XML_Char** attrs) {
                    if (!std::strcmp(element, "osm")) {
                        m_context_stack.push_back(context::osm);
                    } else if (!std::strcmp(element, "osmChange")) {
                        m_context_stack.push_back(context::osmChange);
                        m_header.set_has_multiple_object_versions(true);
                    } else {
                        throw osmium::xml_error{std::string{"Unknown top-level element: "} + element};
                    }

                    check_attributes(attrs, [this](const XML_Char* name, const XML_Char* value) {
                        if (!std::strcmp(name, "version")) {
                            m_header.set("version", value);
                            if (std::strcmp(value, "0.6") != 0) {
                                throw osmium::format_version_error{value};
                            }
                        } else if (!std::strcmp(name, "generator")) {
                            m_header.set("generator", value);
                        } else if (!std::strcmp(name, "upload")) {
                            m_header.set("xml_josm_upload", value);
                        }
                        // ignore other attributes
                    });

                    if (m_header.get("version").empty()) {
                        throw osmium::format_version_error{};
                    }
                }

                void data_level_element(const XML_Char* element, const XML_Char** attrs, bool in_change_section) {
                    assert(!m_node_builder);
                    assert(!m_way_builder);
                    assert(!m_relation_builder);
                    assert(!m_changeset_builder);
                    assert(!m_changeset_discussion_builder);
                    assert(!m_tl_builder);
                    assert(!m_wnl_builder);
                    assert(!m_rml_builder);

                    if (!std::strcmp(element, "node")) {
                        m_context_stack.push_back(context::node);
                        mark_header_as_done();
                        if (read_types() & osmium::osm_entity_bits::node) {
                            maybe_new_buffer(osmium::item_type::node);
                            m_node_builder = std::make_unique<osmium::builder::NodeBuilder>(buffer());
                            m_node_builder->set_user(init_object(m_node_builder->object(), attrs));
                        }
                        return;
                    }

                    if (!std::strcmp(element, "way")) {
                        m_context_stack.push_back(context::way);
                        mark_header_as_done();
                        if (read_types() & osmium::osm_entity_bits::way) {
                            maybe_new_buffer(osmium::item_type::way);
                            m_way_builder = std::make_unique<osmium::builder::WayBuilder>(buffer());
                            m_way_builder->set_user(init_object(m_way_builder->object(), attrs));
                        }
                        return;
                    }

                    if (!std::strcmp(element, "relation")) {
                        m_context_stack.push_back(context::relation);
                        mark_header_as_done();
                        if (read_types() & osmium::osm_entity_bits::relation) {
                            maybe_new_buffer(osmium::item_type::relation);
                            m_relation_builder = std::make_unique<osmium::builder::RelationBuilder>(buffer());
                            m_relation_builder->set_user(init_object(m_relation_builder->object(), attrs));
                        }
                        return;
                    }

                    if (in_change_section) {
                        throw xml_error{"create/modify/delete sections can only contain nodes, ways, and relations"};
                    }

                    if (!std::strcmp(element, "changeset")) {
                        m_context_stack.push_back(context::changeset);
                        mark_header_as_done();
                        if (read_types() & osmium::osm_entity_bits::changeset) {
                            maybe_new_buffer(osmium::item_type::changeset);
                            m_changeset_builder = std::make_unique<osmium::builder::ChangesetBuilder>(buffer());
                            init_changeset(*m_changeset_builder, attrs);
                        }
                    } else if (!std::strcmp(element, "create")) {
                        if (m_context_stack.back() != context::osmChange) {
                            throw xml_error{"<create> only allowed in OSM change files"};
                        }
                        m_context_stack.push_back(context::create_section);
                        mark_header_as_done();
                    } else if (!std::strcmp(element, "modify")) {
                        if (m_context_stack.back() != context::osmChange) {
                            throw xml_error{"<modify> only allowed in OSM change files"};
                        }
                        m_context_stack.push_back(context::modify_section);
                        mark_header_as_done();
                    } else if (!std::strcmp(element, "delete")) {
                        if (m_context_stack.back() != context::osmChange) {
                            throw xml_error{"<delete> only allowed in OSM change files"};
                        }
                        m_context_stack.push_back(context::delete_section);
                        mark_header_as_done();
                    } else if (!std::strcmp(element, "bounds")) {
                        m_context_stack.push_back(context::bounds);
                        osmium::Location min;
                        osmium::Location max;
                        check_attributes(attrs, [&min, &max](const XML_Char* name, const XML_Char* value) {
                            if (!std::strcmp(name, "minlon")) {
                                min.set_lon(value);
                            } else if (!std::strcmp(name, "minlat")) {
                                min.set_lat(value);
                            } else if (!std::strcmp(name, "maxlon")) {
                                max.set_lon(value);
                            } else if (!std::strcmp(name, "maxlat")) {
                                max.set_lat(value);
                            }
                        });
                        osmium::Box box;
                        box.extend(min).extend(max);
                        m_header.add_box(box);
                    } else {
                        m_context_stack.push_back(context::other);
                    }
                }

                void start_element(const XML_Char* element, const XML_Char** attrs) {
                    if (m_context_stack.empty()) {
                        top_level_element(element, attrs);
                        return;
                    }

                    switch (m_context_stack.back()) {
                        case context::osm:
                            // fallthrough
                        case context::osmChange:
                            data_level_element(element, attrs, false);
                            break;
                        case context::create_section:
                            // fallthrough
                        case context::modify_section:
                            // fallthrough
                        case context::delete_section:
                            data_level_element(element, attrs, true);
                            break;
                        case context::node:
                            if (!std::strcmp(element, "tag")) {
                                m_context_stack.push_back(context::tag);
                                if (read_types() & osmium::osm_entity_bits::node) {
                                    get_tag(*m_node_builder, attrs);
                                }
                            } else {
                                throw xml_error{std::string{"Unknown element in <node>: "} + element};
                            }
                            break;
                        case context::way:
                            if (!std::strcmp(element, "nd")) {
                                m_context_stack.push_back(context::nd);
                                if (read_types() & osmium::osm_entity_bits::way) {
                                    m_tl_builder.reset();

                                    if (!m_wnl_builder) {
                                        m_wnl_builder = std::make_unique<osmium::builder::WayNodeListBuilder>(*m_way_builder);
                                    }

                                    NodeRef nr;
                                    check_attributes(attrs, [&nr](const XML_Char* name, const XML_Char* value) {
                                        if (!std::strcmp(name, "ref")) {
                                            nr.set_ref(osmium::string_to_object_id(value));
                                        } else if (!std::strcmp(name, "lon")) {
                                            nr.location().set_lon(value);
                                        } else if (!std::strcmp(name, "lat")) {
                                            nr.location().set_lat(value);
                                        }
                                    });
                                    m_wnl_builder->add_node_ref(nr);
                                }
                            } else if (!std::strcmp(element, "tag")) {
                                m_context_stack.push_back(context::tag);
                                if (read_types() & osmium::osm_entity_bits::way) {
                                    m_wnl_builder.reset();
                                    get_tag(*m_way_builder, attrs);
                                }
                            } else if (!std::strcmp(element, "bbox") || !std::strcmp(element, "bounds")) {
                                m_context_stack.push_back(context::obj_bbox);
                            } else {
                                throw xml_error{std::string{"Unknown element in <way>: "} + element};
                            }
                            break;
                        case context::relation:
                            if (!std::strcmp(element, "member")) {
                                m_context_stack.push_back(context::member);
                                if (read_types() & osmium::osm_entity_bits::relation) {
                                    m_tl_builder.reset();

                                    if (!m_rml_builder) {
                                        m_rml_builder = std::make_unique<osmium::builder::RelationMemberListBuilder>(*m_relation_builder);
                                    }

                                    item_type type = item_type::undefined;
                                    object_id_type ref = 0;
                                    bool ref_is_set = false;
                                    const char* role = "";
                                    check_attributes(attrs, [&type, &ref, &ref_is_set, &role](const XML_Char* name, const XML_Char* value) {
                                        if (!std::strcmp(name, "type")) {
                                            type = char_to_item_type(value[0]);
                                        } else if (!std::strcmp(name, "ref")) {
                                            ref = osmium::string_to_object_id(value);
                                            ref_is_set = true;
                                        } else if (!std::strcmp(name, "role")) {
                                            role = static_cast<const char*>(value);
                                        }
                                    });
                                    if (type != item_type::node && type != item_type::way && type != item_type::relation) {
                                        throw osmium::xml_error{"Unknown type on relation <member>"};
                                    }
                                    if (!ref_is_set) {
                                        throw osmium::xml_error{"Missing ref on relation <member>"};
                                    }
                                    m_rml_builder->add_member(type, ref, role);
                                }
                            } else if (!std::strcmp(element, "tag")) {
                                m_context_stack.push_back(context::tag);
                                if (read_types() & osmium::osm_entity_bits::relation) {
                                    m_rml_builder.reset();
                                    get_tag(*m_relation_builder, attrs);
                                }
                            } else if (!std::strcmp(element, "bbox") || !std::strcmp(element, "bounds")) {
                                m_context_stack.push_back(context::obj_bbox);
                            } else {
                                throw xml_error{std::string{"Unknown element in <relation>: "} + element};
                            }
                            break;
                        case context::tag:
                            throw xml_error{"No element inside <tag> allowed"};
                        case context::nd:
                            throw xml_error{"No element inside <nd> allowed"};
                        case context::member:
                            throw xml_error{"No element inside <member> allowed"};
                        case context::changeset:
                            if (!std::strcmp(element, "discussion")) {
                                m_context_stack.push_back(context::discussion);
                                if (read_types() & osmium::osm_entity_bits::changeset) {
                                    m_tl_builder.reset();
                                    if (!m_changeset_discussion_builder) {
                                        m_changeset_discussion_builder = std::make_unique<osmium::builder::ChangesetDiscussionBuilder>(*m_changeset_builder);
                                    }
                                }
                            } else if (!std::strcmp(element, "tag")) {
                                m_context_stack.push_back(context::tag);
                                if (read_types() & osmium::osm_entity_bits::changeset) {
                                    m_changeset_discussion_builder.reset();
                                    get_tag(*m_changeset_builder, attrs);
                                }
                            } else {
                                throw xml_error{std::string{"Unknown element in <changeset>: "} + element};
                            }
                            break;
                        case context::discussion:
                            if (!std::strcmp(element, "comment")) {
                                m_context_stack.push_back(context::comment);
                                if (read_types() & osmium::osm_entity_bits::changeset) {
                                    osmium::Timestamp date;
                                    osmium::user_id_type uid = 0;
                                    const char* user = "";
                                    check_attributes(attrs, [&date, &uid, &user](const XML_Char* name, const XML_Char* value) {
                                        if (!std::strcmp(name, "date")) {
                                            date = osmium::Timestamp{value};
                                        } else if (!std::strcmp(name, "uid")) {
                                            uid = osmium::string_to_uid(value);
                                        } else if (!std::strcmp(name, "user")) {
                                            user = static_cast<const char*>(value);
                                        }
                                    });
                                    m_changeset_discussion_builder->add_comment(date, uid, user);
                                }
                            } else {
                                throw xml_error{std::string{"Unknown element in <discussion>: "} + element};
                            }
                            break;
                        case context::comment:
                            if (!std::strcmp(element, "text")) {
                                m_context_stack.push_back(context::text);
                            } else {
                                throw xml_error{std::string{"Unknown element in <comment>: "} + element};
                            }
                            break;
                        case context::text:
                            throw osmium::xml_error{"No element in <text> allowed"};
                        case context::bounds:
                            throw osmium::xml_error{"No element in <bounds> allowed"};
                        case context::obj_bbox:
                            throw osmium::xml_error{"No element in <bbox>/<bounds> allowed"};
                        case context::other:
                            throw xml_error{"xml file nested too deep"};
                    }
                }

#ifdef NDEBUG
                void end_element(const XML_Char* /*element*/) {
#else
                void end_element(const XML_Char* element) {
#endif
                    assert(!m_context_stack.empty());
                    switch (m_context_stack.back()) {
                        case context::osm:
                            assert(!std::strcmp(element, "osm"));
                            mark_header_as_done();
                            break;
                        case context::osmChange:
                            assert(!std::strcmp(element, "osmChange"));
                            mark_header_as_done();
                            break;
                        case context::create_section:
                            assert(!std::strcmp(element, "create"));
                            break;
                        case context::modify_section:
                            assert(!std::strcmp(element, "modify"));
                            break;
                        case context::delete_section:
                            assert(!std::strcmp(element, "delete"));
                            break;
                        case context::node:
                            assert(!std::strcmp(element, "node"));
                            if (read_types() & osmium::osm_entity_bits::node) {
                                m_tl_builder.reset();
                                m_node_builder.reset();
                                buffer().commit();
                                flush_nested_buffer();
                            }
                            break;
                        case context::way:
                            assert(!std::strcmp(element, "way"));
                            if (read_types() & osmium::osm_entity_bits::way) {
                                m_tl_builder.reset();
                                m_wnl_builder.reset();
                                m_way_builder.reset();
                                buffer().commit();
                                flush_nested_buffer();
                            }
                            break;
                        case context::relation:
                            assert(!std::strcmp(element, "relation"));
                            if (read_types() & osmium::osm_entity_bits::relation) {
                                m_tl_builder.reset();
                                m_rml_builder.reset();
                                m_relation_builder.reset();
                                buffer().commit();
                                flush_nested_buffer();
                            }
                            break;
                        case context::tag:
                            break;
                        case context::nd:
                            break;
                        case context::member:
                            break;
                        case context::changeset:
                            assert(!std::strcmp(element, "changeset"));
                            if (read_types() & osmium::osm_entity_bits::changeset) {
                                m_tl_builder.reset();
                                m_changeset_discussion_builder.reset();
                                m_changeset_builder.reset();
                                buffer().commit();
                                flush_nested_buffer();
                            }
                            break;
                        case context::discussion:
                            assert(!std::strcmp(element, "discussion"));
                            break;
                        case context::comment:
                            assert(!std::strcmp(element, "comment"));
                            break;
                        case context::text:
                            assert(!std::strcmp(element, "text"));
                            if (read_types() & osmium::osm_entity_bits::changeset) {
                                m_changeset_discussion_builder->add_comment_text(m_comment_text);
                                m_comment_text.clear();
                            }
                            break;
                        case context::bounds:
                            assert(!std::strcmp(element, "bounds"));
                            break;
                        case context::obj_bbox:
                            assert(!std::strcmp(element, "bbox") || !std::strcmp(element, "bounds"));
                            break;
                        case context::other:
                            break;
                    }
                    m_context_stack.pop_back();
                }

                void characters(const XML_Char* text, int len) {
                    if ((read_types() & osmium::osm_entity_bits::changeset) &&
                        !m_context_stack.empty() &&
                        m_context_stack.back() == context::text) {
                        m_comment_text.append(text, len);
                    }
                }

            public:

                explicit XMLParser(parser_arguments& args) :
                    ParserWithBuffer(args) {
                }

                XMLParser(const XMLParser&) = delete;
                XMLParser& operator=(const XMLParser&) = delete;

                XMLParser(XMLParser&&) = delete;
                XMLParser& operator=(XMLParser&&) = delete;

                ~XMLParser() noexcept override = default;

                void run() override {
                    osmium::thread::set_thread_name("_osmium_xml_in");

                    ExpatXMLParser parser{this};
                    m_expat_xml_parser = &parser;

                    while (!input_done()) {
                        const std::string data{get_input()};
                        parser(data, input_done());
                        if (read_types() == osmium::osm_entity_bits::nothing && header_is_done()) {
                            break;
                        }
                    }

                    mark_header_as_done();
                    flush_final_buffer();

                    // so we don't have a dangling link to local parser variable
                    m_expat_xml_parser = nullptr;
                }

            }; // class XMLParser

            // we want the register_parser() function to run, setting
            // the variable is only a side-effect, it will never be used
            const bool registered_xml_parser = ParserFactory::instance().register_parser(
                file_format::xml,
                [](parser_arguments& args) {
                    return std::unique_ptr<Parser>(new XMLParser{args});
            });

            // dummy function to silence the unused variable warning from above
            inline bool get_registered_xml_parser() noexcept {
                return registered_xml_parser;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_XML_INPUT_FORMAT_HPP
