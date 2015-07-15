#ifndef PROTOZERO_PBF_WRITER_HPP
#define PROTOZERO_PBF_WRITER_HPP

/*****************************************************************************

protozero - Minimalistic protocol buffer decoder and encoder in C++.

This file is from https://github.com/mapbox/protozero where you can find more
documentation.

*****************************************************************************/

/**
 * @file pbf_writer.hpp
 *
 * @brief Contains the pbf_writer class.
 */

#if __BYTE_ORDER != __LITTLE_ENDIAN
# error "This code only works on little endian machines."
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>

#include <protozero/pbf_types.hpp>
#include <protozero/varint.hpp>

/// Wrapper for assert() used for testing
#ifndef pbf_assert
# define pbf_assert(x) assert(x)
#endif

namespace protozero {

/**
 * The pbf_writer is used to write PBF formatted messages into a buffer.
 *
 * Almost all methods in this class can throw an std::bad_alloc exception if
 * the std::string used as a buffer wants to resize.
 */
class pbf_writer {

    std::string* m_data;
    pbf_writer* m_parent_writer;
    size_t m_pos = 0;

    inline void add_varint(uint64_t value) {
        pbf_assert(m_pos == 0 && "you can't add fields to a parent pbf_writer if there is an existing pbf_writer for a submessage");
        pbf_assert(m_data);
        write_varint(std::back_inserter(*m_data), value);
    }

    inline void add_field(pbf_tag_type tag, pbf_wire_type type) {
        pbf_assert(((tag > 0 && tag < 19000) || (tag > 19999 && tag <= ((1 << 29) - 1))) && "tag out of range");
        uint32_t b = (tag << 3) | uint32_t(type);
        add_varint(b);
    }

    inline void add_tagged_varint(pbf_tag_type tag, uint64_t value) {
        add_field(tag, pbf_wire_type::varint);
        add_varint(value);
    }

    template <typename T>
    inline void add_fixed(T value) {
        pbf_assert(m_pos == 0 && "you can't add fields to a parent pbf_writer if there is an existing pbf_writer for a submessage");
        pbf_assert(m_data);
        m_data->append(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T, typename It>
    inline void add_packed_fixed(pbf_tag_type tag, It it, It end, std::input_iterator_tag);

    template <typename T, typename It>
    inline void add_packed_fixed(pbf_tag_type tag, It it, It end, std::forward_iterator_tag);

    template <typename It>
    inline void add_packed_varint(pbf_tag_type tag, It begin, It end);

    template <typename It>
    inline void add_packed_svarint(pbf_tag_type tag, It begin, It end);

    // The number of bytes to reserve for the varint holding the length of
    // a length-delimited field. The length has to fit into pbf_length_type,
    // and a varint needs 8 bit for every 7 bit.
    static const int reserve_bytes = sizeof(pbf_length_type) * 8 / 7 + 1;

    inline void open_submessage(pbf_tag_type tag) {
        pbf_assert(m_pos == 0);
        pbf_assert(m_data);
        add_field(tag, pbf_wire_type::length_delimited);
        m_data->append(size_t(reserve_bytes), '\0');
        m_pos = m_data->size();
    }

    inline void close_submessage() {
        pbf_assert(m_pos != 0);
        pbf_assert(m_data);
        auto length = pbf_length_type(m_data->size() - m_pos);

        pbf_assert(m_data->size() >= m_pos - reserve_bytes);
        auto n = write_varint(m_data->begin() + long(m_pos) - reserve_bytes, length);

        m_data->erase(m_data->begin() + long(m_pos) - reserve_bytes + n, m_data->begin() + long(m_pos));
        m_pos = 0;
    }

public:

    /**
     * Create a writer using the given string as a data store. The pbf_writer
     * stores a reference to that string and adds all data to it.
     */
    inline explicit pbf_writer(std::string& data) noexcept :
        m_data(&data),
        m_parent_writer(nullptr),
        m_pos(0) {
    }

    /**
     * Create a writer without a data store. In this form the writer can not
     * be used!
     */
    inline pbf_writer() noexcept :
        m_data(nullptr),
        m_parent_writer(nullptr),
        m_pos(0) {
    }

    /**
     * Construct a pbf_writer for a submessage from the pbf_writer of the
     * parent message.
     *
     * @param parent_writer The pbf_writer
     * @param tag Tag (field number) of the field that will be written
     */
    inline pbf_writer(pbf_writer& parent_writer, pbf_tag_type tag) :
        m_data(parent_writer.m_data),
        m_parent_writer(&parent_writer),
        m_pos(0) {
        m_parent_writer->open_submessage(tag);
    }

    /// A pbf_writer object can be copied
    pbf_writer(const pbf_writer&) noexcept = default;

    /// A pbf_writer object can be copied
    pbf_writer& operator=(const pbf_writer&) noexcept = default;

    /// A pbf_writer object can be moved
    inline pbf_writer(pbf_writer&&) noexcept = default;

    /// A pbf_writer object can be moved
    inline pbf_writer& operator=(pbf_writer&&) noexcept = default;

    inline ~pbf_writer() {
        if (m_parent_writer) {
            m_parent_writer->close_submessage();
        }
    }

    inline void add_length_varint(pbf_tag_type tag, pbf_length_type value) {
        add_field(tag, pbf_wire_type::length_delimited);
        add_varint(value);
    }

    inline void append(const char* value, size_t size) {
        pbf_assert(m_pos == 0 && "you can't add fields to a parent pbf_writer if there is an existing pbf_writer for a submessage");
        pbf_assert(m_data);
        m_data->append(value, size);
    }

    ///@{
    /**
     * @name Scalar field writer functions
     */

    /**
     * Add "bool" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_bool(pbf_tag_type tag, bool value) {
        add_field(tag, pbf_wire_type::varint);
        add_fixed<char>(value);
    }

    /**
     * Add "enum" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_enum(pbf_tag_type tag, int32_t value) {
        add_tagged_varint(tag, uint64_t(value));
    }

    /**
     * Add "int32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_int32(pbf_tag_type tag, int32_t value) {
        add_tagged_varint(tag, uint64_t(value));
    }

    /**
     * Add "sint32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sint32(pbf_tag_type tag, int32_t value) {
        add_tagged_varint(tag, encode_zigzag32(value));
    }

    /**
     * Add "uint32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_uint32(pbf_tag_type tag, uint32_t value) {
        add_tagged_varint(tag, value);
    }

    /**
     * Add "int64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_int64(pbf_tag_type tag, int64_t value) {
        add_tagged_varint(tag, uint64_t(value));
    }

    /**
     * Add "sint64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sint64(pbf_tag_type tag, int64_t value) {
        add_tagged_varint(tag, encode_zigzag64(value));
    }

    /**
     * Add "uint64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_uint64(pbf_tag_type tag, uint64_t value) {
        add_tagged_varint(tag, value);
    }

    /**
     * Add "fixed32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_fixed32(pbf_tag_type tag, uint32_t value) {
        add_field(tag, pbf_wire_type::fixed32);
        add_fixed<uint32_t>(value);
    }

    /**
     * Add "sfixed32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sfixed32(pbf_tag_type tag, int32_t value) {
        add_field(tag, pbf_wire_type::fixed32);
        add_fixed<int32_t>(value);
    }

    /**
     * Add "fixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_fixed64(pbf_tag_type tag, uint64_t value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_fixed<uint64_t>(value);
    }

    /**
     * Add "sfixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sfixed64(pbf_tag_type tag, int64_t value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_fixed<int64_t>(value);
    }

    /**
     * Add "float" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_float(pbf_tag_type tag, float value) {
        add_field(tag, pbf_wire_type::fixed32);
        add_fixed<float>(value);
    }

    /**
     * Add "double" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_double(pbf_tag_type tag, double value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_fixed<double>(value);
    }

    /**
     * Add "bytes" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Pointer to value to be written
     * @param size Number of bytes to be written
     */
    inline void add_bytes(pbf_tag_type tag, const char* value, size_t size) {
        add_field(tag, pbf_wire_type::length_delimited);
        add_varint(size);
        append(value, size);
    }

    /**
     * Add "bytes" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_bytes(pbf_tag_type tag, const std::string& value) {
        add_bytes(tag, value.data(), value.size());
    }

    /**
     * Add "string" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Pointer to value to be written
     * @param size Number of bytes to be written
     */
    inline void add_string(pbf_tag_type tag, const char* value, size_t size) {
        add_bytes(tag, value, size);
    }

    /**
     * Add "string" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_string(pbf_tag_type tag, const std::string& value) {
        add_bytes(tag, value.data(), value.size());
    }

    /**
     * Add "string" field to data. Bytes from the value are written until
     * a null byte is encountered. The null byte is not added.
     *
     * @param tag Tag (field number) of the field
     * @param value Pointer to value to be written
     */
    inline void add_string(pbf_tag_type tag, const char* value) {
        add_bytes(tag, value, std::strlen(value));
    }

    /**
     * Add "message" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written. The value must be a complete message.
     */
    inline void add_message(pbf_tag_type tag, const std::string& value) {
        add_bytes(tag, value.data(), value.size());
    }

    ///@}

    ///@{
    /**
     * @name Repeated packed field writer functions
     */

    /**
     * Add "repeated packed fixed32" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to uint32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_fixed32(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_fixed<uint32_t, InputIterator>(tag, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
    }

    /**
     * Add "repeated packed fixed64" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to uint64_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_fixed64(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_fixed<uint64_t, InputIterator>(tag, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
    }

    /**
     * Add "repeated packed sfixed32" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_sfixed32(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_fixed<int32_t, InputIterator>(tag, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
    }

    /**
     * Add "repeated packed sfixed64" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int64_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_sfixed64(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_fixed<int64_t, InputIterator>(tag, begin, end, typename std::iterator_traits<InputIterator>::iterator_category());
    }

    /**
     * Add "repeated packed bool" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to bool.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_bool(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed enum" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_enum(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed int32" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_int32(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed uint32" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to uint32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_uint32(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed sint32" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int32_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_sint32(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_svarint(tag, begin, end);
    }

    /**
     * Add "repeated packed int64" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int64_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_int64(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed uint64" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to uint64_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_uint64(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed sint64" field to data.
     *
     * @tparam InputIterator An type satisfying the InputIterator concept.
     *         Dereferencing the iterator must yield a type assignable to int64_t.
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename InputIterator>
    inline void add_packed_sint64(pbf_tag_type tag, InputIterator begin, InputIterator end) {
        add_packed_svarint(tag, begin, end);
    }

    ///@}

}; // class pbf_writer

template <typename T, typename It>
inline void pbf_writer::add_packed_fixed(pbf_tag_type tag, It it, It end, std::forward_iterator_tag) {
    if (it == end) {
        return;
    }

    add_field(tag, pbf_wire_type::length_delimited);
    add_varint(sizeof(T) * pbf_length_type(std::distance(it, end)));

    while (it != end) {
        add_fixed<T>(*it++);
    }
}

template <typename T, typename It>
inline void pbf_writer::add_packed_fixed(pbf_tag_type tag, It it, It end, std::input_iterator_tag) {
    if (it == end) {
        return;
    }

    pbf_writer sw(*this, tag);

    while (it != end) {
        sw.add_fixed<T>(*it++);
    }
}

template <typename It>
inline void pbf_writer::add_packed_varint(pbf_tag_type tag, It it, It end) {
    if (it == end) {
        return;
    }

    pbf_writer sw(*this, tag);

    while (it != end) {
        sw.add_varint(uint64_t(*it++));
    }
}

template <typename It>
inline void pbf_writer::add_packed_svarint(pbf_tag_type tag, It it, It end) {
    if (it == end) {
        return;
    }

    pbf_writer sw(*this, tag);

    while (it != end) {
        sw.add_varint(encode_zigzag64(*it++));
    }
}

} // end namespace protozero

#endif // PROTOZERO_PBF_WRITER_HPP
