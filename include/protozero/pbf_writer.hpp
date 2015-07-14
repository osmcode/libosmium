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
 * All methods in this class can throw an exception if the std::string used
 * as a buffer throws an exception. This can only happen if we are out of
 * memory and the string wants to resize.
 */
class pbf_writer {

    std::string& m_data;

    inline void add_varint(uint64_t value) {
        write_varint(std::back_inserter(m_data), value);
    }

    inline void add_tagged_varint(pbf_tag_type tag, uint64_t value) {
        add_field(tag, pbf_wire_type::varint);
        add_varint(value);
    }

    inline void add_field(pbf_tag_type tag, pbf_wire_type type) {
        pbf_assert(((tag > 0 && tag < 19000) || (tag > 19999 && tag <= ((1 << 29) - 1))) && "tag out of range");
        uint32_t b = (tag << 3) | uint32_t(type);
        add_varint(b);
    }

    template <typename T>
    inline void add_data(T value) {
        m_data.append(reinterpret_cast<const char*>(&value), sizeof(T));
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

    inline void reserve_space() {
        m_data.append(size_t(reserve_bytes), '\0');
    }

public:

    /**
     * Create a writer using the given string as a data store. The pbf_writer
     * stores a reference to that string and adds all data to it.
     */
    inline pbf_writer(std::string& data) :
        m_data(data) {
    }

    /// A pbf_writer object can not be copied (because of the reference inside)
    pbf_writer(const pbf_writer&) = delete;

    /// A pbf_writer object can not be copied (because of the reference inside)
    pbf_writer& operator=(const pbf_writer&) = delete;

    /// A pbf_writer object can be moved
    inline pbf_writer(pbf_writer&&) noexcept = default;

    /// A pbf_writer object can be moved
    inline pbf_writer& operator=(pbf_writer&&) noexcept = default;

    inline ~pbf_writer() noexcept = default;

    inline void add_length_varint(pbf_tag_type tag, pbf_length_type value) {
        add_field(tag, pbf_wire_type::length_delimited);
        add_varint(value);
    }

    inline void append(const char* value, size_t size) {
        m_data.append(value, size);
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
        m_data.append(1, char(value));
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
        add_data<uint32_t>(value);
    }

    /**
     * Add "sfixed32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sfixed32(pbf_tag_type tag, int32_t value) {
        add_field(tag, pbf_wire_type::fixed32);
        add_data<int32_t>(value);
    }

    /**
     * Add "fixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_fixed64(pbf_tag_type tag, uint64_t value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_data<uint64_t>(value);
    }

    /**
     * Add "sfixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_sfixed64(pbf_tag_type tag, int64_t value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_data<int64_t>(value);
    }

    /**
     * Add "float" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_float(pbf_tag_type tag, float value) {
        add_field(tag, pbf_wire_type::fixed32);
        add_data<float>(value);
    }

    /**
     * Add "double" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_double(pbf_tag_type tag, double value) {
        add_field(tag, pbf_wire_type::fixed64);
        add_data<double>(value);
    }

    /**
     * Add "bytes" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_bytes(pbf_tag_type tag, const std::string& value) {
        add_field(tag, pbf_wire_type::length_delimited);
        add_varint(value.size());
        m_data.append(value);
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
        m_data.append(value, size);
    }

    /**
     * Add "string" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written
     */
    inline void add_string(pbf_tag_type tag, const std::string& value) {
        add_bytes(tag, value);
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
     * Add "string" field to data. Bytes from the value are written until
     * a null byte is encountered. The null byte is not added.
     *
     * @param tag Tag (field number) of the field
     * @param value Pointer to value to be written
     */
    inline void add_string(pbf_tag_type tag, const char* value) {
        add_field(tag, pbf_wire_type::length_delimited);
        add_varint(std::strlen(value));
        m_data.append(value);
    }

    /**
     * Add "message" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param value Value to be written. The value must be a complete message.
     */
    inline void add_message(pbf_tag_type tag, const std::string& value) {
        add_bytes(tag, value);
    }

    ///@}

    ///@{
    /**
     * @name Repeated packed field writer functions
     */

    /**
     * Add "repeated packed fixed32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_fixed32(pbf_tag_type tag, It begin, It end) {
        add_packed_fixed<uint32_t>(tag, begin, end, typename std::iterator_traits<It>::iterator_category());
    }

    /**
     * Add "repeated packed fixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_fixed64(pbf_tag_type tag, It begin, It end) {
        add_packed_fixed<uint64_t>(tag, begin, end, typename std::iterator_traits<It>::iterator_category());
    }

    /**
     * Add "repeated packed sfixed32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_sfixed32(pbf_tag_type tag, It begin, It end) {
        add_packed_fixed<int32_t>(tag, begin, end, typename std::iterator_traits<It>::iterator_category());
    }

    /**
     * Add "repeated packed sfixed64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_sfixed64(pbf_tag_type tag, It begin, It end) {
        add_packed_fixed<int64_t>(tag, begin, end, typename std::iterator_traits<It>::iterator_category());
    }

    /**
     * Add "repeated packed bool" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_bool(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed enum" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_enum(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed int32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_int32(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed uint32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_uint32(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed sint32" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_sint32(pbf_tag_type tag, It begin, It end) {
        add_packed_svarint(tag, begin, end);
    }

    /**
     * Add "repeated packed int64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_int64(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed uint64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_uint64(pbf_tag_type tag, It begin, It end) {
        add_packed_varint(tag, begin, end);
    }

    /**
     * Add "repeated packed sint64" field to data.
     *
     * @param tag Tag (field number) of the field
     * @param begin Iterator pointing to the beginning of the data
     * @param end Iterator pointing one past the end of data
     */
    template <typename It>
    inline void add_packed_sint64(pbf_tag_type tag, It begin, It end) {
        add_packed_svarint(tag, begin, end);
    }

    ///@}

    ///@{
    /**
     * @name Functions for adding length-delimited fields.
     */

    /**
     * Open a length-delimited field. Can be used together with close_sub()
     * and append_sub() to fill a length-delimited field without knowing
     * beforehand how large it is going to be.
     *
     * You can use these functions like this:
     *
     * @code
     * std::string data;
     * pbf_writer w(data);
     * auto pos = w.open_sub(23);
     * w.append_sub("foo");
     * w.append_sub("bar");
     * w.close_sub(pos);
     * @endcode
     *
     * Instead of using open_sub(), close_sub(), and append_sub() it is
     * recommended to use the pbf_subwriter class which encapsulates this
     * functionality in a nice wrapper.
     *
     * @param tag Tag (field number) of the field
     * @returns The position in the data.
     */
    inline size_t open_sub(pbf_tag_type tag) {
        add_field(tag, pbf_wire_type::length_delimited);
        reserve_space();
        return m_data.size();
    }

    /**
     * Close length-delimited field previously opened with open_sub().
     *
     * @param pos The position in the data returned by open_sub().
     */
    inline void close_sub(size_t pos) {
        auto length = pbf_length_type(m_data.size() - pos);

        pbf_assert(m_data.size() >= pos - reserve_bytes);
        auto n = write_varint(&m_data[pos - reserve_bytes], length);

        m_data.erase(m_data.begin() + long(pos) - reserve_bytes + n, m_data.begin() + long(pos));
    }

    /**
     * Append some data to a length-delimited field opened with open_sub().
     *
     * @param value The data to be added
     */
    void append_sub(const std::string& value) {
        m_data.append(value);
    }

    /**
     * Append some data to a length-delimited field opened with open_sub().
     *
     * @param value Pointer to the data to be added
     * @param size The length of the data
     */
    void append_sub(const char* value, size_t size) {
        m_data.append(value, size);
    }

    ///@}

}; // class pbf_writer

/**
 * Wrapper class for the pbf_writer open_sub(), close_sub(), and append_sub()
 * functions.
 *
 * Usage:
 * @code
 * std::string data;
 * pbf_writer w(data);
 * {
 *     pbf_subwriter s(w, 23);
 *     s.append("foo");
 *     s.append("bar");
 * } // field is automatically closed
 * @endcode
 */
class pbf_subwriter {

    pbf_writer& m_writer;
    size_t m_pos = 0;

public:

    /**
     * Construct a pbf_subwriter from a pbf_writer.
     *
     * @param writer The pbf_writer
     * @param tag Tag (field number) of the field that will be written
     */
    inline pbf_subwriter(pbf_writer& writer, pbf_tag_type tag) :
        m_writer(writer),
        m_pos(writer.open_sub(tag)) {
    }

    /// A pbf_subwriter can not be copied.
    pbf_subwriter(const pbf_subwriter&) = delete;

    /// A pbf_subwriter can not be copied.
    pbf_subwriter& operator=(const pbf_subwriter&) = delete;

    /// A pbf_subwriter can not be moved.
    pbf_subwriter(pbf_subwriter&&) = delete;

    /// A pbf_subwriter can not be moved.
    pbf_subwriter& operator=(pbf_subwriter&&) = delete;

    /**
     * Destroying a pbf_subwriter will update the pbf_writer it was constructed
     * with. It is important the destructor is called before any other
     * operations are done on the pbf_writer.
     */
    inline ~pbf_subwriter() {
        m_writer.close_sub(m_pos);
    }

    /**
     * Append data to the pbf message.
     *
     * @param value Value to be written
     */
    inline void append(const std::string& value) {
        m_writer.append_sub(value);
    }

    /**
     * Append data to the pbf message.
     *
     * @param value Pointer to value to be written
     * @param size Number of bytes to be written
     */
    inline void append(const char* value, size_t size) {
        m_writer.append_sub(value, size);
    }

}; // class pbf_subwriter

template <typename T>
class pbf_appender : public std::iterator<std::output_iterator_tag, T> {

    pbf_writer* m_writer;

public:

    pbf_appender(pbf_writer& writer, pbf_tag_type tag, pbf_length_type size) :
        m_writer(&writer) {
        writer.add_length_varint(tag, size * sizeof(T));
    }

    pbf_appender& operator=(const T value) {
        m_writer->append(reinterpret_cast<const char*>(&value), sizeof(T));
        return *this;
    }

    pbf_appender& operator*() {
        // do nothing
        return *this;
    }

    pbf_appender& operator++() {
        // do nothing
        return *this;
    }

    pbf_appender& operator++(int) {
        // do nothing
        return *this;
    }

}; // class pbf_appender

template <typename T, typename It>
inline void pbf_writer::add_packed_fixed(pbf_tag_type tag, It it, It end, std::forward_iterator_tag) {
    if (it == end) {
        return;
    }

    add_field(tag, pbf_wire_type::length_delimited);
    add_varint(sizeof(T) * pbf_length_type(std::distance(it, end)));

    while (it != end) {
        const T v = *it++;
        m_data.append(reinterpret_cast<const char*>(&v), sizeof(T));
    }
}

template <typename T, typename It>
inline void pbf_writer::add_packed_fixed(pbf_tag_type tag, It it, It end, std::input_iterator_tag) {
    if (it == end) {
        return;
    }

    pbf_subwriter sw(*this, tag);

    while (it != end) {
        const T v = *it++;
        m_data.append(reinterpret_cast<const char*>(&v), sizeof(T));
    }
}

template <typename It>
inline void pbf_writer::add_packed_varint(pbf_tag_type tag, It it, It end) {
    if (it == end) {
        return;
    }

    pbf_subwriter sw(*this, tag);

    while (it != end) {
        add_varint(uint64_t(*it++));
    }
}

template <typename It>
inline void pbf_writer::add_packed_svarint(pbf_tag_type tag, It it, It end) {
    if (it == end) {
        return;
    }

    pbf_subwriter sw(*this, tag);

    while (it != end) {
        add_varint(encode_zigzag64(*it++));
    }
}

} // end namespace protozero

#endif // PROTOZERO_PBF_WRITER_HPP
