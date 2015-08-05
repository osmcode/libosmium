#ifndef OSMIUM_IO_DETAIL_PBF_DECODER_HPP
#define OSMIUM_IO_DETAIL_PBF_DECODER_HPP

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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iterator>

#include <protozero/pbf_reader.hpp>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/io/detail/pbf.hpp> // IWYU pragma: export
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

            using ptr_len_type = std::pair<const char*, size_t>;

            class PBFPrimitiveBlockDecoder {

                static constexpr size_t initial_buffer_size = 2 * 1024 * 1024;

                ptr_len_type m_data;
                std::vector<ptr_len_type> m_stringtable;

                int64_t m_lon_offset = 0;
                int64_t m_lat_offset = 0;
                int64_t m_date_factor = 1000;
                int32_t m_granularity = 100;

                osmium::osm_entity_bits::type m_read_types;

                osmium::memory::Buffer m_buffer { initial_buffer_size };

                void decode_stringtable(const ptr_len_type& data) {
                    if (!m_stringtable.empty()) {
                        throw osmium::pbf_error("more than one stringtable in pbf file");
                    }

                    protozero::pbf_reader pbf_string_table(data);
                    while (pbf_string_table.next(1 /* repeated bytes s*/)) {
                        m_stringtable.push_back(pbf_string_table.get_data());
                    }
                }

                void decode_primitive_block_metadata() {
                    protozero::pbf_reader pbf_primitive_block(m_data);
                    while (pbf_primitive_block.next()) {
                        switch (pbf_primitive_block.tag()) {
                            case 1: // required StringTable stringtable
                                decode_stringtable(pbf_primitive_block.get_data());
                                break;
                            case 17: // optional int32 granularity
                                m_granularity = pbf_primitive_block.get_int32();
                                break;
                            case 18: // optional int32 date_granularity
                                m_date_factor = pbf_primitive_block.get_int32();
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
                    protozero::pbf_reader pbf_primitive_block(m_data);
                    while (pbf_primitive_block.next(2 /* repeated PrimitiveGroup primitivegroup */)) {
                        auto pbf_primitive_group = pbf_primitive_block.get_message();
                        while (pbf_primitive_group.next()) {
                            switch (pbf_primitive_group.tag()) {
                                case 1: // repeated Node nodes
                                    if (m_read_types & osmium::osm_entity_bits::node) {
                                        decode_node(pbf_primitive_group.get_data());
                                    } else {
                                        pbf_primitive_group.skip();
                                    }
                                    break;
                                case 2: // optional DenseNodes dense
                                    if (m_read_types & osmium::osm_entity_bits::node) {
                                        decode_dense_nodes(pbf_primitive_group.get_data());
                                    } else {
                                        pbf_primitive_group.skip();
                                    }
                                    break;
                                case 3: // repeated Way ways
                                    if (m_read_types & osmium::osm_entity_bits::way) {
                                        decode_way(pbf_primitive_group.get_data());
                                    } else {
                                        pbf_primitive_group.skip();
                                    }
                                    break;
                                case 4: // repeated Relation relations
                                    if (m_read_types & osmium::osm_entity_bits::relation) {
                                        decode_relation(pbf_primitive_group.get_data());
                                    } else {
                                        pbf_primitive_group.skip();
                                    }
                                    break;
                                default:
                                    pbf_primitive_group.skip();
                            }
                        }
                    }
                }

                ptr_len_type decode_info(const ptr_len_type& data, osmium::OSMObject& object) {
                    auto user = std::make_pair<const char*, size_t>("", 0);

                    protozero::pbf_reader pbf_info(data);
                    while (pbf_info.next()) {
                        switch (pbf_info.tag()) {
                            case 1: // optional int32 version
                                {
                                    auto version = pbf_info.get_int32();
                                    if (version < 0) {
                                        throw osmium::pbf_error("object version must not be negative");
                                    }
                                    object.set_version(static_cast_with_assert<object_version_type>(version));
                                }
                                break;
                            case 2: // optional int64 timestamp
                                object.set_timestamp(pbf_info.get_int64() * m_date_factor / 1000);
                                break;
                            case 3: // optional int64 changeset
                                {
                                    auto changeset_id = pbf_info.get_int64();
                                    if (changeset_id < 0) {
                                        throw osmium::pbf_error("object changeset_id must not be negative");
                                    }
                                    object.set_changeset(static_cast_with_assert<changeset_id_type>(changeset_id));
                                }
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

                using kv_type = std::pair<protozero::pbf_reader::const_uint32_iterator, protozero::pbf_reader::const_uint32_iterator>;

                void build_tag_list(osmium::builder::Builder& builder, const kv_type& keys, const kv_type& vals) {
                    if (keys.first != keys.second) {
                        osmium::builder::TagListBuilder tl_builder(m_buffer, &builder);
                        auto kit = keys.first;
                        auto vit = vals.first;
                        while (kit != keys.second) {
                            if (vit == vals.second) {
                                // this is against the spec, must have same number of elements
                                throw osmium::pbf_error("PBF format error");
                            }
                            const auto& k = m_stringtable.at(*kit++);
                            const auto& v = m_stringtable.at(*vit++);
                            tl_builder.add_tag(k.first, k.second, v.first, v.second);
                        }
                    }
                }

                int32_t convert_pbf_coordinate(int64_t c) const {
                    return (c * m_granularity + m_lon_offset) / resolution_convert;
                }

                void decode_node(const ptr_len_type& data) {
                    osmium::builder::NodeBuilder builder(m_buffer);
                    osmium::Node& node = builder.object();

                    kv_type keys;
                    kv_type vals;
                    int64_t lon;
                    int64_t lat;

                    ptr_len_type user = { "", 0 };

                    protozero::pbf_reader pbf_node(data);
                    while (pbf_node.next()) {
                        switch (pbf_node.tag()) {
                            case 1: // required sint64 id
                                node.set_id(pbf_node.get_sint64());
                                break;
                            case 2: // repeated uint32 keys [packed = true]
                                keys = pbf_node.get_packed_uint32();
                                break;
                            case 3: // repeated uint32 vals [packed = true]
                                vals = pbf_node.get_packed_uint32();
                                break;
                            case 4: // Optional Info info
                                user = decode_info(pbf_node.get_data(), builder.object());
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

                    if (node.visible()) {
                        node.set_location(osmium::Location(
                                convert_pbf_coordinate(lon),
                                convert_pbf_coordinate(lat)
                        ));
                    }

                    builder.add_user(user.first, user.second);

                    build_tag_list(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_way(const ptr_len_type& data) {
                    osmium::builder::WayBuilder builder(m_buffer);

                    kv_type keys;
                    kv_type vals;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> refs;

                    ptr_len_type user = { "", 0 };

                    protozero::pbf_reader pbf_way(data);
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
                                user = decode_info(pbf_way.get_data(), builder.object());
                                break;
                            case 8: // repeated sint64 refs [packed = true] DELTA encoded
                                refs = pbf_way.get_packed_sint64();
                                break;
                            default:
                                pbf_way.skip();
                        }
                    }

                    builder.add_user(user.first, user.second);

                    if (refs.first != refs.second) {
                        osmium::builder::WayNodeListBuilder wnl_builder(m_buffer, &builder);
                        osmium::util::DeltaDecode<int64_t> ref;
                        while (refs.first != refs.second) {
                            wnl_builder.add_node_ref(ref.update(*refs.first++));
                        }
                    }

                    build_tag_list(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_relation(const ptr_len_type& data) {
                    osmium::builder::RelationBuilder builder(m_buffer);

                    kv_type keys;
                    kv_type vals;
                    std::pair<protozero::pbf_reader::const_int32_iterator,  protozero::pbf_reader::const_int32_iterator> roles;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> refs;
                    std::pair<protozero::pbf_reader::const_int32_iterator,  protozero::pbf_reader::const_int32_iterator> types;

                    ptr_len_type user = { "", 0 };

                    protozero::pbf_reader pbf_relation(data);
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
                                user = decode_info(pbf_relation.get_data(), builder.object());
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

                    builder.add_user(user.first, user.second);

                    if (refs.first != refs.second) {
                        osmium::builder::RelationMemberListBuilder rml_builder(m_buffer, &builder);
                        osmium::util::DeltaDecode<int64_t> ref;
                        while (roles.first != roles.second && refs.first != refs.second && types.first != types.second) {
                            const auto& r = m_stringtable.at(*roles.first++);
                            int type = *types.first++;
                            if (type < 0 || type > 2) {
                                throw osmium::pbf_error("unknown relation member type");
                            }
                            rml_builder.add_member(
                                osmium::item_type(type + 1),
                                ref.update(*refs.first++),
                                r.first,
                                r.second
                            );
                        }
                    }

                    build_tag_list(builder, keys, vals);

                    m_buffer.commit();
                }

                void decode_dense_nodes(const ptr_len_type& data) {
                    bool has_info     = false;
                    bool has_visibles = false;

                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> ids;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> lats;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> lons;

                    std::pair<protozero::pbf_reader::const_int32_iterator,  protozero::pbf_reader::const_int32_iterator>  tags;

                    std::pair<protozero::pbf_reader::const_int32_iterator,  protozero::pbf_reader::const_int32_iterator>  versions;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> timestamps;
                    std::pair<protozero::pbf_reader::const_sint64_iterator, protozero::pbf_reader::const_sint64_iterator> changesets;
                    std::pair<protozero::pbf_reader::const_sint32_iterator, protozero::pbf_reader::const_sint32_iterator> uids;
                    std::pair<protozero::pbf_reader::const_sint32_iterator, protozero::pbf_reader::const_sint32_iterator> user_sids;
                    std::pair<protozero::pbf_reader::const_int32_iterator,  protozero::pbf_reader::const_int32_iterator>  visibles;

                    protozero::pbf_reader pbf_dense_nodes(data);
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

                    osmium::util::DeltaDecode<int64_t> dense_id;
                    osmium::util::DeltaDecode<int64_t> dense_latitude;
                    osmium::util::DeltaDecode<int64_t> dense_longitude;
                    osmium::util::DeltaDecode<int64_t> dense_uid;
                    osmium::util::DeltaDecode<int64_t> dense_user_sid;
                    osmium::util::DeltaDecode<int64_t> dense_changeset;
                    osmium::util::DeltaDecode<int64_t> dense_timestamp;

                    auto tag_it = tags.first;

                    while (ids.first != ids.second) {
                        if (lons.first == lons.second ||
                            lats.first == lats.second) {
                            // this is against the spec, must have same number of elements
                            throw osmium::pbf_error("PBF format error");
                        }

                        bool visible = true;

                        osmium::builder::NodeBuilder builder(m_buffer);
                        osmium::Node& node = builder.object();

                        node.set_id(dense_id.update(*ids.first++));

                        if (has_info) {
                            if (versions.first == versions.second ||
                                changesets.first == changesets.second ||
                                timestamps.first == timestamps.second ||
                                uids.first == uids.second ||
                                user_sids.first == user_sids.second) {
                                // this is against the spec, must have same number of elements
                                throw osmium::pbf_error("PBF format error");
                            }

                            auto version = *versions.first++;
                            if (version < 0) {
                                throw osmium::pbf_error("object version must not be negative");
                            }
                            node.set_version(static_cast<osmium::object_version_type>(version));

                            auto changeset_id = dense_changeset.update(*changesets.first++);
                            if (changeset_id < 0) {
                                throw osmium::pbf_error("object changeset_id must not be negative");
                            }
                            node.set_changeset(static_cast<osmium::changeset_id_type>(changeset_id));

                            node.set_timestamp(dense_timestamp.update(*timestamps.first++) * m_date_factor / 1000);
                            node.set_uid_from_signed(static_cast<osmium::signed_user_id_type>(dense_uid.update(*uids.first++)));

                            if (has_visibles) {
                                if (visibles.first == visibles.second) {
                                    // this is against the spec, must have same number of elements
                                    throw osmium::pbf_error("PBF format error");
                                }
                                visible = *visibles.first++;
                            }
                            node.set_visible(visible);

                            const auto& u = m_stringtable.at(dense_user_sid.update(*user_sids.first++));
                            builder.add_user(u.first, u.second);
                        } else {
                            builder.add_user("");
                        }

                        if (visible) {
                            builder.object().set_location(osmium::Location(
                                    convert_pbf_coordinate(dense_longitude.update(*lons.first++)),
                                    convert_pbf_coordinate(dense_latitude.update(*lats.first++))
                            ));
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

                explicit PBFPrimitiveBlockDecoder(const ptr_len_type& data, osmium::osm_entity_bits::type read_types) :
                    m_data(data),
                    m_read_types(read_types) {
                }

                PBFPrimitiveBlockDecoder(const PBFPrimitiveBlockDecoder&) = delete;
                PBFPrimitiveBlockDecoder& operator=(const PBFPrimitiveBlockDecoder&) = delete;

                PBFPrimitiveBlockDecoder(PBFPrimitiveBlockDecoder&&) = delete;
                PBFPrimitiveBlockDecoder& operator=(PBFPrimitiveBlockDecoder&&) = delete;

                ~PBFPrimitiveBlockDecoder() = default;

                osmium::memory::Buffer operator()() {
                    try {
                        decode_primitive_block_metadata();
                        decode_primitive_block_data();
                    } catch (std::out_of_range&) {
                        throw osmium::pbf_error("string id out of range");
                    }

                    return std::move(m_buffer);
                }

            }; // class PBFPrimitiveBlockDecoder

            inline ptr_len_type decode_blob(const std::string& blob_data, std::string& output) {
                int32_t raw_size;
                std::pair<const char*, protozero::pbf_length_type> zlib_data;

                protozero::pbf_reader pbf_blob(blob_data);
                while (pbf_blob.next()) {
                    switch (pbf_blob.tag()) {
                        case 1: // optional bytes raw
                            {
                                auto data_len = pbf_blob.get_data();
                                if (data_len.second > max_uncompressed_blob_size) {
                                    throw osmium::pbf_error("illegal blob size");
                                }
                                return data_len;
                            }
                        case 2: // optional int32 raw_size
                            raw_size = pbf_blob.get_int32();
                            if (raw_size <= 0 || uint32_t(raw_size) > max_uncompressed_blob_size) {
                                throw osmium::pbf_error("illegal blob size");
                            }
                            break;
                        case 3: // optional bytes zlib_data
                            zlib_data = pbf_blob.get_data();
                            break;
                        case 4: // optional bytes lzma_data
                            throw osmium::pbf_error("lzma blobs not implemented");
                        default:
                            throw osmium::pbf_error("unknown compression");
                    }
                }

                if (zlib_data.second != 0) {
                    return osmium::io::detail::zlib_uncompress_string(
                        zlib_data.first,
                        static_cast<unsigned long>(zlib_data.second),
                        static_cast<unsigned long>(raw_size),
                        output
                    );
                }

                throw osmium::pbf_error("blob contains no data");
            }

            inline osmium::Box decode_header_bbox(const ptr_len_type& data) {
                    int64_t left, right, top, bottom;

                    protozero::pbf_reader pbf_header_bbox(data);
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

                    return box;
            }

            inline osmium::io::Header decode_header_block(const ptr_len_type& data) {
                osmium::io::Header header;
                int i = 0;

                protozero::pbf_reader pbf_header_block(data);
                while (pbf_header_block.next()) {
                    switch (pbf_header_block.tag()) {
                        case 1: // optional HeaderBBox bbox
                            header.add_box(decode_header_bbox(pbf_header_block.get_data()));
                            break;
                        case 4: // repeated string required_features
                            {
                                auto feature = pbf_header_block.get_data();
                                if (!strncmp("OsmSchema-V0.6", feature.first, feature.second)) {
                                    // intentionally left blank
                                } else if (!strncmp("DenseNodes", feature.first, feature.second)) {
                                    header.set("pbf_dense_nodes", true);
                                } else if (!strncmp("HistoricalInformation", feature.first, feature.second)) {
                                    header.set_has_multiple_object_versions(true);
                                } else {
                                    std::string msg("required feature not supported: ");
                                    msg.append(feature.first, feature.second);
                                    throw osmium::pbf_error(msg);
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

            /**
             * Decode HeaderBlock.
             *
             * @param header_block_data Input data
             * @returns Header object
             * @throws osmium::pbf_error If there was a parsing error
             */
            inline osmium::io::Header decode_header(const std::string& header_block_data) {
                std::string output;

                return decode_header_block(decode_blob(header_block_data, output));
            }

            class PBFDataBlobDecoder {

                std::shared_ptr<std::string> m_input_buffer;
                osmium::osm_entity_bits::type m_read_types;

            public:

                PBFDataBlobDecoder(std::string&& input_buffer, osmium::osm_entity_bits::type read_types) :
                    m_input_buffer(std::make_shared<std::string>(std::move(input_buffer))),
                    m_read_types(read_types) {
                }

                PBFDataBlobDecoder(const PBFDataBlobDecoder&) = default;
                PBFDataBlobDecoder& operator=(const PBFDataBlobDecoder&) = default;

                PBFDataBlobDecoder(PBFDataBlobDecoder&&) = default;
                PBFDataBlobDecoder& operator=(PBFDataBlobDecoder&&) = default;

                ~PBFDataBlobDecoder() = default;

                osmium::memory::Buffer operator()() {
                    std::string output;
                    PBFPrimitiveBlockDecoder decoder(decode_blob(*m_input_buffer, output), m_read_types);
                    return decoder();
                }

            }; // class PBFDataBlobDecoder

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_PBF_DECODER_HPP
