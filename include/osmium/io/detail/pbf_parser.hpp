#ifndef OSMIUM_IO_DETAIL_PBF_PRIMITIVE_BLOCK_PARSER_HPP
#define OSMIUM_IO_DETAIL_PBF_PRIMITIVE_BLOCK_PARSER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2015 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iterator>

#include <pbf_reader.hpp>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/detail/pbf.hpp> // IWYU pragma: export
#include <osmium/io/detail/pbf_type_conv.hpp>
#include <osmium/io/detail/zlib.hpp>
#include <osmium/io/header.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/types.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/util/cast.hpp>
#include <osmium/util/delta.hpp>

namespace osmium {

    namespace io {

        namespace detail {

            class PBFPrimitiveBlockParser {

                static constexpr size_t initial_buffer_size = 2 * 1024 * 1024;

                std::pair<const char*, size_t> m_data;

                std::vector<std::pair<const char*, size_t>> m_stringtable;

                int64_t m_lon_offset;
                int64_t m_lat_offset;
                int64_t m_date_factor;
                int32_t m_granularity;

                osmium::osm_entity_bits::type m_read_types;

                osmium::memory::Buffer m_buffer;

                void decode_stringtable(mapbox::util::pbf&& pbf_string_table) {
                    if (!m_stringtable.empty()) {
                        throw osmium::pbf_error("more than one stringtable in pbf file");
                    }
                    while (pbf_string_table.next(1 /* repeated bytes s*/)) {
                        m_stringtable.push_back(pbf_string_table.get_data());
                    }
                }

                void decode_primitive_block_metadata() {
                    mapbox::util::pbf pbf_primitive_block(m_data);

                    while (pbf_primitive_block.next()) {
                        switch (pbf_primitive_block.tag()) {
                            case 1: // required StringTable stringtable
                                decode_stringtable(std::move(pbf_primitive_block.get_message()));
                                break;
                            case 17: // optional int32 granularity
                                m_granularity = pbf_primitive_block.get_int32();
                                break;
                            case 18: // optional int32 date_granularity
                                m_date_factor = pbf_primitive_block.get_int32() / 1000;
                                break;
                            case 19: // optional int64 lat_offset
                                m_lat_offset = pbf_primitive_block.get_int64();
                                break;
                            case 20: // optional int64 lon_offset
                                m_lon_offset = pbf_primitive_block.get_int64();
                                break;
                            default:
                                pbf_primitive_block.skip();
                        }
                    }
                }

                void decode_primitive_block_data() {
                    mapbox::util::pbf pbf_primitive_block(m_data);

                    while (pbf_primitive_block.next(2 /* repeated PrimitiveGroup primitivegroup */)) {
                        auto pbf_primitive_group = pbf_primitive_block.get_message();
                        while (pbf_primitive_group.next()) {
                            switch (pbf_primitive_group.tag()) {
                                case 1: // repeated Node nodes
                                    if (m_read_types & osmium::osm_entity_bits::node) {
                                        decode_node(std::move(pbf_primitive_group.get_message()));
                                    }
                                    break;
                                case 2: // optional DenseNodes dense
                                    if (m_read_types & osmium::osm_entity_bits::node) {
                                        decode_dense_nodes(std::move(pbf_primitive_group.get_message()));
                                    }
                                    break;
                                case 3: // repeated Way ways
                                    if (m_read_types & osmium::osm_entity_bits::way) {
                                        decode_way(std::move(pbf_primitive_group.get_message()));
                                    }
                                    break;
                                case 4: // repeated Relation relations
                                    if (m_read_types & osmium::osm_entity_bits::relation) {
                                        decode_relation(std::move(pbf_primitive_group.get_message()));
                                    }
                                    break;
                                default:
                                    pbf_primitive_group.skip();
                            }
                        }
                    }
                }

                std::pair<const char*, size_t> decode_info(osmium::OSMObject& object, mapbox::util::pbf&& pbf_info) {
                    auto user = std::make_pair<const char*, size_t>("", 0);

                    while (pbf_info.next()) {
                        switch (pbf_info.tag()) {
                            case 1: // optional int32 version
                                object.set_version(static_cast_with_assert<object_version_type>(pbf_info.get_int32()));
                                break;
                            case 2: // optional int64 timestamp
                                object.set_timestamp(pbf_info.get_int64() * m_date_factor);
                                break;
                            case 3: // optional int64 changeset
                                object.set_changeset(static_cast_with_assert<changeset_id_type>(pbf_info.get_int64()));
                                break;
                            case 4: // optional int32 uid
                                object.set_uid_from_signed(pbf_info.get_int32());
                                break;
                            case 5: // optional uint32 user_sid
                                user = m_stringtable.at(pbf_info.get_uint32());
                                break;
                            case 6: // optional bool visible
                                object.set_visible(pbf_info.get_bool());
                                break;
                            default:
                                pbf_info.skip();
                        }
                    }

                    return user;
                }


                using kv_type = std::pair<mapbox::util::pbf::const_uint32_iterator, mapbox::util::pbf::const_uint32_iterator>;

                void decode_tags(osmium::builder::Builder& builder, const kv_type& keys, const kv_type& vals) {
                    if (keys.first != keys.second) {
                        osmium::builder::TagListBuilder tl_builder(m_buffer, &builder);
                        auto kit = keys.first;
                        auto vit = vals.first;
                        while (kit != keys.second && vit != vals.second) {
                            const auto& k = m_stringtable.at(*kit++);
                            const auto& v = m_stringtable.at(*vit++);
                            tl_builder.add_tag(k.first, k.second, v.first, v.second);
                        }
                    }
                }

                void decode_node(mapbox::util::pbf&& pbf_node) {
                    osmium::builder::NodeBuilder builder(m_buffer);

                    kv_type keys;
                    kv_type vals;

                    int64_t lat;
                    int64_t lon;
                    while (pbf_node.next()) {
                        switch (pbf_node.tag()) {
                            case 1: // required sint64 id
                                builder.object().set_id(pbf_node.get_sint64());
                                break;
                            case 2: // repeated uint32 keys [packed = true]
                                keys = pbf_node.get_packed_uint32();
                                break;
                            case 3: // repeated uint32 vals [packed = true]
                                vals = pbf_node.get_packed_uint32();
                                break;
                            case 4: // Optional Info info
                                {
                                    const auto u = decode_info(builder.object(), std::move(pbf_node.get_message()));
                                    builder.add_user(u.first, u.second);
                                }
                                break;
                            case 8: // required sint64 lat
                                lat = pbf_node.get_sint64();
                                break;
                            case 9: // required sint64 lon
                                lon = pbf_node.get_sint64();
                                break;
                            default:
                                pbf_node.skip();
                        }
                    }

                    if (builder.object().visible()) {
                        builder.object().set_location(osmium::Location(
                                            (lon * m_granularity + m_lon_offset) / (lonlat_resolution / osmium::Location::coordinate_precision),
                                            (lat * m_granularity + m_lat_offset) / (lonlat_resolution / osmium::Location::coordinate_precision)));
                    }

                    decode_tags(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_way(mapbox::util::pbf&& pbf_way) {
                    osmium::builder::WayBuilder builder(m_buffer);

                    kv_type keys;
                    kv_type vals;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> refs;

                    while (pbf_way.next()) {
                        switch (pbf_way.tag()) {
                            case 1: // required int64 id
                                builder.object().set_id(pbf_way.get_int64());
                                break;
                            case 2: // repeated uint32 keys [packed = true]
                                keys = pbf_way.get_packed_uint32();
                                break;
                            case 3: // repeated uint32 vals [packed = true]
                                vals = pbf_way.get_packed_uint32();
                                break;
                            case 4: // optional Info info
                                {
                                    const auto u = decode_info(builder.object(), std::move(pbf_way.get_message()));
                                    builder.add_user(u.first, u.second);
                                }
                                break;
                            case 8: // repeated sint64 refs [packed = true] DELTA encoded
                                refs = pbf_way.get_packed_sint64();
                                break;
                            default:
                                pbf_way.skip();
                        }
                    }

                    if (refs.first != refs.second) {
                        osmium::builder::WayNodeListBuilder wnl_builder(m_buffer, &builder);
                        osmium::util::DeltaDecode<int64_t> ref;
                        auto it = refs.first;
                        while (it != refs.second) {
                            wnl_builder.add_node_ref(ref.update(*it++));
                        }
                    }

                    decode_tags(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_relation(mapbox::util::pbf&& pbf_relation) {
                    osmium::builder::RelationBuilder builder(m_buffer);

                    kv_type keys;
                    kv_type vals;
                    std::pair<mapbox::util::pbf::const_int32_iterator,  mapbox::util::pbf::const_int32_iterator> roles;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> refs;
                    std::pair<mapbox::util::pbf::const_int32_iterator,  mapbox::util::pbf::const_int32_iterator> types;

                    while (pbf_relation.next()) {
                        switch (pbf_relation.tag()) {
                            case 1: // required int64 id
                                builder.object().set_id(pbf_relation.get_int64());
                                break;
                            case 2: // repeated uint32 keys [packed = true]
                                keys = pbf_relation.get_packed_uint32();
                                break;
                            case 3: // repeated uint32 vals [packed = true]
                                vals = pbf_relation.get_packed_uint32();
                                break;
                            case 4: // optional Info info
                                {
                                    const auto u = decode_info(builder.object(), std::move(pbf_relation.get_message()));
                                    builder.add_user(u.first, u.second);
                                }
                                break;
                            case 8: // repeated int32 roles_sid [packed = true]
                                roles = pbf_relation.get_packed_int32();
                                break;
                            case 9: // repeated sint64 memids [packed = true] DELTA encoded
                                refs = pbf_relation.get_packed_sint64();
                                break;
                            case 10: // repeated MemberType types [packed = true]
                                types = pbf_relation.get_packed_enum();
                                break;
                            default:
                                pbf_relation.skip();
                        }
                    }

                    if (refs.first != refs.second) {
                        osmium::builder::RelationMemberListBuilder rml_builder(m_buffer, &builder);
                        osmium::util::DeltaDecode<int64_t> ref;
                        auto roles_it = roles.first;
                        auto ref_it = refs.first;
                        auto type_it = types.first;
                        while (roles_it != roles.second && ref_it != refs.second && type_it != types.second) {
                            const auto& r = m_stringtable.at(*roles_it++);
                            rml_builder.add_member(
                                osmium::item_type(*type_it++ + 1),
                                ref.update(*ref_it++),
                                r.first,
                                r.second
                            );
                        }
                    }

                    decode_tags(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_dense_nodes(mapbox::util::pbf&& pbf_dense_nodes) {
                    bool has_info     = false;
                    bool has_visibles = false;

                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> ids;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> lats;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> lons;
                    std::pair<mapbox::util::pbf::const_int32_iterator,  mapbox::util::pbf::const_int32_iterator>  tags;

                    std::pair<mapbox::util::pbf::const_int32_iterator,  mapbox::util::pbf::const_int32_iterator>  versions;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> timestamps;
                    std::pair<mapbox::util::pbf::const_sint64_iterator, mapbox::util::pbf::const_sint64_iterator> changesets;
                    std::pair<mapbox::util::pbf::const_sint32_iterator, mapbox::util::pbf::const_sint32_iterator> uids;
                    std::pair<mapbox::util::pbf::const_sint32_iterator, mapbox::util::pbf::const_sint32_iterator> user_sids;
                    std::pair<mapbox::util::pbf::const_int32_iterator,  mapbox::util::pbf::const_int32_iterator>  visibles;

                    while (pbf_dense_nodes.next()) {
                        switch (pbf_dense_nodes.tag()) {
                            case 1: // repeated sint64 id [packed = true] DELTA encoded
                                ids = pbf_dense_nodes.get_packed_sint64();
                                break;
                            case 5: // optional DenseInfo denseinfo
                                {
                                    has_info = true;
                                    auto pbf_dense_info = pbf_dense_nodes.get_message();
                                    while (pbf_dense_info.next()) {
                                        switch (pbf_dense_info.tag()) {
                                            case 1: // repeated int32 version [packed = true]
                                                versions = pbf_dense_info.get_packed_int32();
                                                break;
                                            case 2: // repeated sint64 timestamp [packed = true] DELTA encoded
                                                timestamps = pbf_dense_info.get_packed_sint64();
                                                break;
                                            case 3: // repeated sint64 changeset [packed = true] DELTA encoded
                                                changesets = pbf_dense_info.get_packed_sint64();
                                                break;
                                            case 4: // repeated sint32 uid [packed = true] DELTA encoded
                                                uids = pbf_dense_info.get_packed_sint32();
                                                break;
                                            case 5: // repeated sint32 user_sid [packed = true] DELTA encoded
                                                user_sids = pbf_dense_info.get_packed_sint32();
                                                break;
                                            case 6: // repeated bool visible [packed = true]
                                                has_visibles = true;
                                                visibles = pbf_dense_info.get_packed_bool();
                                                break;
                                            default:
                                                pbf_dense_info.skip();
                                        }
                                    }
                                }
                                break;
                            case 8: // repeated sint64 lat [packed = true] DELTA encoded
                                lats = pbf_dense_nodes.get_packed_sint64();
                                break;
                            case 9: // repeated sint64 lon [packed = true] DELTA coded
                                lons = pbf_dense_nodes.get_packed_sint64();
                                break;
                            case 10: // repeated int32 keys_vals [packed = true]
                                tags = pbf_dense_nodes.get_packed_int32();
                                break;
                            default:
                                pbf_dense_nodes.skip();
                        }
                    }

                    auto count = std::distance(ids.first, ids.second);
                    assert(count == std::distance(lats.first, lats.second));
                    assert(count == std::distance(lons.first, lons.second));

                    if (has_info) {
                        assert(count == std::distance(versions.first, versions.second));
                        assert(count == std::distance(timestamps.first, timestamps.second));
                        assert(count == std::distance(changesets.first, changesets.second));
                        assert(count == std::distance(uids.first, uids.second));
                        assert(count == std::distance(user_sids.first, user_sids.second));

                        if (has_visibles) {
                            assert(count == std::distance(visibles.first, visibles.second));
                        }
                    }

                    osmium::util::DeltaDecode<int64_t> dense_id;
                    osmium::util::DeltaDecode<int64_t> dense_latitude;
                    osmium::util::DeltaDecode<int64_t> dense_longitude;
                    osmium::util::DeltaDecode<int64_t> dense_uid;
                    osmium::util::DeltaDecode<int64_t> dense_user_sid;
                    osmium::util::DeltaDecode<int64_t> dense_changeset;
                    osmium::util::DeltaDecode<int64_t> dense_timestamp;

                    auto id_it  = ids.first;
                    auto lat_it = lats.first;
                    auto lon_it = lons.first;

                    auto version_it   = versions.first;
                    auto timestamp_it = timestamps.first;
                    auto changeset_it = changesets.first;
                    auto uid_it       = uids.first;
                    auto user_sid_it  = user_sids.first;
                    auto visible_it   = visibles.first;
                    auto tag_it       = tags.first;

                    while (id_it != ids.second) {
                        bool visible = true;

                        osmium::builder::NodeBuilder builder(m_buffer);
                        osmium::Node& node = builder.object();

                        node.set_id(dense_id.update(*id_it++));

                        if (has_info) {
                            node.set_version(static_cast<osmium::object_version_type>(*version_it++));
                            node.set_changeset(static_cast<osmium::changeset_id_type>(dense_changeset.update(*changeset_it++)));
                            node.set_timestamp(dense_timestamp.update(*timestamp_it++) * m_date_factor);
                            node.set_uid_from_signed(static_cast<osmium::signed_user_id_type>(dense_uid.update(*uid_it++)));

                            if (has_visibles) {
                                visible = *visible_it++;
                            }
                            node.set_visible(visible);

                            const auto& u = m_stringtable.at(dense_user_sid.update(*user_sid_it++));
                            builder.add_user(u.first, u.second);
                        } else {
                            builder.add_user("");
                        }

                        if (visible) {
                            builder.object().set_location(osmium::Location(
                                              (dense_longitude.update(*lon_it++) * m_granularity + m_lon_offset) / (lonlat_resolution / osmium::Location::coordinate_precision),
                                              (dense_latitude.update(*lat_it++)  * m_granularity + m_lat_offset) / (lonlat_resolution / osmium::Location::coordinate_precision)));
                        }

                        if (tag_it != tags.second) {
                            osmium::builder::TagListBuilder tl_builder(m_buffer, &builder);
                            while (tag_it != tags.second && *tag_it != 0) {
                                const auto& k = m_stringtable.at(*tag_it++);
                                if (tag_it == tags.second) {
                                    throw osmium::pbf_error("PBF format error"); // this is against the spec, keys/vals must come in pairs
                                }
                                const auto& v = m_stringtable.at(*tag_it++);
                                tl_builder.add_tag(k.first, k.second, v.first, v.second);
                            }

                            if (tag_it != tags.second) {
                                ++tag_it;
                            }
                        }

                        m_buffer.commit();
                    }

                }

            public:

                explicit PBFPrimitiveBlockParser(const std::pair<const char*, size_t>& data, osmium::osm_entity_bits::type read_types) :
                    m_data(data),
                    m_stringtable(),
                    m_lon_offset(0),
                    m_lat_offset(0),
                    m_date_factor(1),
                    m_granularity(100),
                    m_read_types(read_types),
                    m_buffer(initial_buffer_size) {
                }

                PBFPrimitiveBlockParser(const PBFPrimitiveBlockParser&) = delete;
                PBFPrimitiveBlockParser& operator=(const PBFPrimitiveBlockParser&) = delete;

                PBFPrimitiveBlockParser(PBFPrimitiveBlockParser&&) = delete;
                PBFPrimitiveBlockParser& operator=(PBFPrimitiveBlockParser&&) = delete;

                ~PBFPrimitiveBlockParser() = default;

                osmium::memory::Buffer operator()() {
                    decode_primitive_block_metadata();
                    decode_primitive_block_data();

                    return std::move(m_buffer);
                }

            }; // class PBFPrimitiveBlockParser

            /**
             * PBF blobs can optionally be packed with the zlib algorithm.
             * This function returns the raw data (if it was unpacked) or
             * the unpacked data (if it was packed).
             *
             * @param input_data Reference to input data.
             * @returns Unpacked data
             * @throws osmium::pbf_error If there was a problem parsing the PBF
             */
            inline std::pair<const char*, size_t> unpack_blob(const std::string& input_data, std::string& output) {
                mapbox::util::pbf pbf_blob(input_data);

                bool has_zlib = false;
                int32_t raw_size;
                std::string zlib_data;
                while (pbf_blob.next()) {
                    switch (pbf_blob.tag()) {
                        case 1: // optional bytes raw
                            return pbf_blob.get_data();
                        case 2: // optional int32 raw_size
                            raw_size = pbf_blob.get_int32();
                            break;
                        case 3: // optional bytes zlib_data
                            zlib_data = std::move(pbf_blob.get_bytes());
                            has_zlib = true;
                            break;
                        case 4: // optional bytes lzma_data
                            throw osmium::pbf_error("lzma blobs not implemented");
                        default:
                            throw osmium::pbf_error("unknown compression");
                    }
                }

                if (has_zlib) {
                    assert(raw_size >= 0);
                    assert(raw_size <= max_uncompressed_blob_size);
                    return osmium::io::detail::zlib_uncompress_string(zlib_data, static_cast<unsigned long>(raw_size), output);
                } else {
                    throw osmium::pbf_error("blob contains no data");
                }
            }

            inline void parse_header_bbox(osmium::io::Header& header, mapbox::util::pbf&& pbf_header_bbox) {
                    const int64_t resolution_convert = lonlat_resolution / osmium::Location::coordinate_precision;

                    int64_t left, right, top, bottom;

                    while (pbf_header_bbox.next()) {
                        switch (pbf_header_bbox.tag()) {
                            case 1: // required sint64 left
                                left = pbf_header_bbox.get_sint64();
                                break;
                            case 2: // required sint64 right
                                right = pbf_header_bbox.get_sint64();
                                break;
                            case 3: // required sint64 top
                                top = pbf_header_bbox.get_sint64();
                                break;
                            case 4: // required sint64 bottom
                                bottom = pbf_header_bbox.get_sint64();
                                break;
                            default:
                                pbf_header_bbox.skip();
                        }
                    }

                    osmium::Box box;
                    box.extend(osmium::Location(left  / resolution_convert, bottom / resolution_convert));
                    box.extend(osmium::Location(right / resolution_convert, top    / resolution_convert));
                    header.add_box(box);
            }

            /**
             * Parse blob as a HeaderBlock.
             *
             * @param input_buffer Blob data
             * @returns Header object
             * @throws osmium::pbf_error If there was a parsing error
             */
            inline osmium::io::Header parse_header_blob(const std::string& input_buffer) {
                std::string output;
                mapbox::util::pbf pbf_header_block(unpack_blob(input_buffer, output));

                osmium::io::Header header;
                int i = 0;
                while (pbf_header_block.next()) {
                    switch (pbf_header_block.tag()) {
                        case 1: // optional HeaderBBox bbox
                            parse_header_bbox(header, pbf_header_block.get_message());
                            break;
                        case 4: // repeated string required_features
                            {
                                const std::string& feature = pbf_header_block.get_string();
                                if (feature == "OsmSchema-V0.6") {
                                    // intentionally left blank
                                } else if (feature == "DenseNodes") {
                                    header.set("pbf_dense_nodes", true);
                                } else if (feature == "HistoricalInformation") {
                                    header.set_has_multiple_object_versions(true);
                                } else {
                                    throw osmium::pbf_error(std::string("required feature not supported: ") + feature);
                                }
                            }
                            break;
                        case 5: // repeated string optional_features
                            header.set("pbf_optional_feature_" + std::to_string(i++), pbf_header_block.get_string());
                            break;
                        case 16: // optional string writingprogram
                            header.set("generator", pbf_header_block.get_string());
                            break;
                        case 32: // optional int64 osmosis_replication_timestamp
                            header.set("osmosis_replication_timestamp", osmium::Timestamp(pbf_header_block.get_int64()).to_iso());
                            break;
                        case 33: // optional int64 osmosis_replication_sequence_number
                            header.set("osmosis_replication_sequence_number", std::to_string(pbf_header_block.get_int64()));
                            break;
                        case 34: // optional string osmosis_replication_base_url
                            header.set("osmosis_replication_base_url", pbf_header_block.get_string());
                            break;
                        default:
                            pbf_header_block.skip();
                    }
                }

                return header;
            }

            class DataBlobParser {

                std::shared_ptr<std::string> m_input_buffer;
                osmium::osm_entity_bits::type m_read_types;

            public:

                DataBlobParser(std::string&& input_buffer, osmium::osm_entity_bits::type read_types) :
                    m_input_buffer(std::make_shared<std::string>(std::move(input_buffer))),
                    m_read_types(read_types) {
                }

                DataBlobParser(const DataBlobParser&) = default;
                DataBlobParser& operator=(const DataBlobParser&) = default;

                DataBlobParser(DataBlobParser&&) = default;
                DataBlobParser& operator=(DataBlobParser&&) = default;

                ~DataBlobParser() = default;

                osmium::memory::Buffer operator()() {
                    std::string output;
                    try {
                        PBFPrimitiveBlockParser parser(unpack_blob(*m_input_buffer, output), m_read_types);
                        return parser();
                    } catch (std::out_of_range&) {
                        throw osmium::pbf_error("string id out of range");
                    }
                }

            }; // class DataBlobParser

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_PBF_PRIMITIVE_BLOCK_PARSER_HPP
