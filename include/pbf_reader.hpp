#ifndef MAPBOX_UTIL_PBF_READER_HPP
#define MAPBOX_UTIL_PBF_READER_HPP

/*****************************************************************************

Minimalistic fast C++ decoder for a subset of the protocol buffer format.

This is header-only, meaning there is nothing to build. Just include this file
in your C++ application.

This file is from https://github.com/mapbox/pbf.hpp where you can find more
documentation.

*****************************************************************************/

#if __BYTE_ORDER != __LITTLE_ENDIAN
# error "This code only works on little endian machines."
#endif

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iterator>
#include <string>
#include <utility>

#include "pbf_common.hpp"

#ifndef pbf_assert
# define pbf_assert(x) assert(x)
#endif

namespace mapbox { namespace util {

/**
 * This class represents a protobuf message. Either a top-level message or
 * a nested sub-message. Top-level messages can be created from any buffer
 * with a pointer and length:
 *
 * @code
 *    std::string buffer;
 *    // fill buffer...
 *    pbf message(buffer.data(), buffer.size());
 * @endcode
 *
 * Sub-messages are created using get_message():
 *
 * @code
 *    pbf message(...);
 *    message.next();
 *    pbf submessage = message.get_message();
 * @endcode
 *
 */
class pbf {

    // The maximum length of a 64bit varint.
    static const int8_t max_varint_length = sizeof(uint64_t) * 8 / 7 + 1;

    // from https://github.com/facebook/folly/blob/master/folly/Varint.h
    static uint64_t decode_varint(const char** data, const char* end) {
        const int8_t* begin = reinterpret_cast<const int8_t*>(*data);
        const int8_t* iend = reinterpret_cast<const int8_t*>(end);
        const int8_t* p = begin;
        uint64_t val = 0;

        if (iend - begin >= max_varint_length) {  // fast path
            do {
                int64_t b;
                b = *p++; val  = uint64_t((b & 0x7f)      ); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) <<  7); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 14); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 21); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 28); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 35); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 42); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 49); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 56); if (b >= 0) break;
                b = *p++; val |= uint64_t((b & 0x7f) << 63); if (b >= 0) break;
                throw varint_too_long_exception();
            } while (false);
        } else {
            int shift = 0;
            while (p != iend && *p < 0) {
                val |= uint64_t(*p++ & 0x7f) << shift;
                shift += 7;
            }
            if (p == iend) {
                throw end_of_buffer_exception();
            }
            val |= uint64_t(*p++) << shift;
        }

        *data = reinterpret_cast<const char*>(p);
        return val;
    }

    // A pointer to the next unread data.
    const char *m_data = nullptr;

    // A pointer to one past the end of data.
    const char *m_end = nullptr;

    // The wire type of the current field.
    pbf_wire_type m_wire_type = pbf_wire_type::unknown;

    // The tag of the current field.
    pbf_tag_type m_tag = 0;

    template <typename T> inline T get_fixed();
    template <typename T> inline T get_varint();
    template <typename T> inline T get_svarint();
    template <typename T> inline std::pair<const T*, const T*> packed_fixed();

    inline pbf_length_type get_length() { return get_varint<pbf_length_type>(); }

    inline void skip_bytes(pbf_length_type len);

    inline pbf_length_type get_len_and_skip();

public:

    /**
     * Decodes a 32 bit ZigZag-encoded integer.
     *
     * This is a helper function used inside the pbf class, but could be
     * useful in other contexts.
     */
    static inline int32_t decode_zigzag32(uint32_t value) noexcept;

    /**
     * Decodes a 64 bit ZigZag-encoded integer.
     *
     * This is a helper function used inside the pbf class, but could be
     * useful in other contexts.
     */
    static inline int64_t decode_zigzag64(uint64_t value) noexcept;

    /**
     * All exceptions thrown by the functions of the pbf class derive from
     * this exception.
     */
    struct exception : std::exception {
        /// Returns the explanatory string.
        const char *what() const noexcept { return "pbf exception"; }
    };

    /**
     * This exception is thrown when parsing a varint thats larger than allowed.
     * This should never happen unless the data is corrupted.
     * After the exception the pbf object is in an unknown
     * state and you cannot recover from that.
     */
    struct varint_too_long_exception : exception {
        /// Returns the explanatory string.
        const char *what() const noexcept { return "pbf varint too long exception"; }
    };

    /**
     * This exception is thrown when the type of a field is unknown.
     * This should never happen unless the data is corrupted.
     * After the exception the pbf object is in an unknown
     * state and you cannot recover from that.
     */
    struct unknown_field_type_exception : exception {
        /// Returns the explanatory string.
        const char *what() const noexcept { return "pbf unknown field type exception"; }
    };

    /**
     * This exception is thrown when we are trying to read a field and there
     * are not enough bytes left in the buffer to read it. Almost all functions
     * can throw this exception.
     * This should never happen unless the data is corrupted or you have
     * initialized the pbf object with incomplete data.
     * After the exception the pbf object is in an unknown
     * state and you cannot recover from that.
     */
    struct end_of_buffer_exception : exception {
        /// Returns the explanatory string.
        const char *what() const noexcept { return "pbf end of buffer exception"; }
    };

    /**
     * Construct a pbf message from a data pointer and a length. The pointer
     * will be stored inside the pbf object, no data is copied. So you must
     * make sure the buffer stays valid as long as the pbf object is used.
     *
     * The buffer must contain a complete protobuf message.
     *
     * @post There is no current field.
     */
    inline pbf(const char *data, size_t length);

    inline pbf(const std::string& data);

    inline pbf() = default;

    /// pbf messages can be copied trivially.
    inline pbf(const pbf&) = default;

    /// pbf messages can be moved trivially.
    inline pbf(pbf&&) = default;

    /// pbf messages can be copied trivially.
    inline pbf& operator=(const pbf& other) = default;

    /// pbf messages can be moved trivially.
    inline pbf& operator=(pbf&& other) = default;

    inline ~pbf() = default;

    /**
     * In a boolean context the pbf class evaluates to `true` if there are
     * still fields available and to `false` if the last field has been read.
     */
    inline operator bool() const noexcept;

    /**
     * Return the length in bytes of the current message. If you have
     * already called next() and/or any of the get_*() functions, this will
     * return the remaining length.
     *
     * This can, for instance, be used to estimate the space needed for a
     * buffer. Of course you have to know reasonably well what data to expect
     * and how it is encoded for this number to have any meaning.
     */
    size_t length() const noexcept {
        return size_t(m_end - m_data);
    }

    /**
     * Set next field in the message as the current field. This is usually
     * called in a while loop:
     *
     * @code
     *    pbf message(...);
     *    while (message.next()) {
     *        // handle field
     *    }
     * @endcode
     *
     * @returns `true` if there is a next field, `false` if not.
     * @pre There must be no current field.
     * @post If it returns `true` there is a current field now.
     */
    inline bool next();

    /**
     * Set next field with given tag in the message as the current field.
     * Fields with other tags are skipped. This is usually called in a while
     * loop for repeated fields:
     *
     * @code
     *    pbf message(...);
     *    while (message.next(17)) {
     *        // handle field
     *    }
     * @endcode
     *
     * or you can call it just once to get the one field with this tag:
     *
     * @code
     *    pbf message(...);
     *    if (message.next(17)) {
     *        // handle field
     *    }
     * @endcode
     *
     * @returns `true` if there is a next field with this tag.
     * @pre There must be no current field.
     * @post If it returns `true` there is a current field now with the given tag.
     */
    inline bool next(pbf_tag_type tag);

    /**
     * The tag of the current field. The tag is the field number from the
     * description in the .proto file.
     *
     * Call next() before calling this function to set the current field.
     *
     * @returns tag of the current field.
     * @pre There must be a current field (ie. next() must have returned `true`).
     */
    inline pbf_tag_type tag() const noexcept;

    /**
     * Get the wire type of the current field. The wire types are:
     *
     * * 0 - varint
     * * 1 - 64 bit
     * * 2 - length-delimited
     * * 5 - 32 bit
     *
     * All other types are illegal.
     *
     * Call next() before calling this function to set the current field.
     *
     * @returns wire type of the current field.
     * @pre There must be a current field (ie. next() must have returned `true`).
     */
    inline pbf_wire_type wire_type() const noexcept;

    /**
     * Check the wire type of the current field.
     *
     * @returns `true` if the current field has the given wire type.
     * @pre There must be a current field (ie. next() must have returned `true`).
     */
    inline bool has_wire_type(pbf_wire_type type) const noexcept;

    /**
     * Consume the current field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @post The current field was consumed and there is no current field now.
     */
    inline void skip();

    ///@{
    /**
     * @name Scalar field accessor functions
     */

    /**
     * Consume and return value of current "bool" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "bool".
     * @post The current field was consumed and there is no current field now.
     */
    inline bool get_bool();

    /**
     * Consume and return value of current "enum" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "enum".
     * @post The current field was consumed and there is no current field now.
     */
    inline int32_t get_enum() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_varint<int32_t>();
    }

    /**
     * Consume and return value of current "int32" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "int32".
     * @post The current field was consumed and there is no current field now.
     */
    inline int32_t get_int32() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_varint<int32_t>();
    }

    /**
     * Consume and return value of current "sint32" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "sint32".
     * @post The current field was consumed and there is no current field now.
     */
    inline int32_t get_sint32() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_svarint<int32_t>();
    }

    /**
     * Consume and return value of current "uint32" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "uint32".
     * @post The current field was consumed and there is no current field now.
     */
    inline uint32_t get_uint32() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_varint<uint32_t>();
    }

    /**
     * Consume and return value of current "int64" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "int64".
     * @post The current field was consumed and there is no current field now.
     */
    inline int64_t get_int64() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_varint<int64_t>();
    }

    /**
     * Consume and return value of current "sint64" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "sint64".
     * @post The current field was consumed and there is no current field now.
     */
    inline int64_t get_sint64() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_svarint<int64_t>();
    }

    /**
     * Consume and return value of current "uint64" varint field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "uint64".
     * @post The current field was consumed and there is no current field now.
     */
    inline uint64_t get_uint64() {
        pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
        return get_varint<uint64_t>();
    }

    /**
     * Consume and return value of current "fixed32" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "fixed32".
     * @post The current field was consumed and there is no current field now.
     */
    inline uint32_t get_fixed32();

    /**
     * Consume and return value of current "sfixed32" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "sfixed32".
     * @post The current field was consumed and there is no current field now.
     */
    inline int32_t get_sfixed32();

    /**
     * Consume and return value of current "fixed64" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "fixed64".
     * @post The current field was consumed and there is no current field now.
     */
    inline uint64_t get_fixed64();

    /**
     * Consume and return value of current "sfixed64" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "sfixed64".
     * @post The current field was consumed and there is no current field now.
     */
    inline int64_t get_sfixed64();

    /**
     * Consume and return value of current "float" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "float".
     * @post The current field was consumed and there is no current field now.
     */
    inline float get_float();

    /**
     * Consume and return value of current "double" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "double".
     * @post The current field was consumed and there is no current field now.
     */
    inline double get_double();

    /**
     * Consume and return value of current "bytes" or "string" field.
     *
     * @returns A pair with a pointer to the data and the length of the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "bytes" or "string".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<const char*, pbf_length_type> get_data();

    /**
     * Consume and return value of current "bytes" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "bytes".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::string get_bytes();

    /**
     * Consume and return value of current "string" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "string".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::string get_string();

    /**
     * Consume and return value of current "message" field.
     *
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "message".
     * @post The current field was consumed and there is no current field now.
     */
    inline pbf get_message();

    ///@}

private:

    template <typename T>
    class const_varint_iterator : public std::iterator<std::forward_iterator_tag, T> {

    protected:

        const char* m_data;
        const char* m_end;

    public:

        const_varint_iterator() noexcept :
            m_data(nullptr),
            m_end(nullptr) {
        }

        const_varint_iterator(const char *data, const char* end) noexcept :
            m_data(data),
            m_end(end) {
        }

        const_varint_iterator(const const_varint_iterator&) noexcept = default;
        const_varint_iterator(const_varint_iterator&&) noexcept = default;

        const_varint_iterator& operator=(const const_varint_iterator&) noexcept = default;
        const_varint_iterator& operator=(const_varint_iterator&&) noexcept = default;

        ~const_varint_iterator() noexcept = default;

        T operator*() {
            const char* d = m_data; // will be thrown away
            return static_cast<T>(decode_varint(&d, m_end));
        }

        const_varint_iterator& operator++() {
            // Ignore the result, we call decode_varint() just for the
            // side-effect of updating data.
            decode_varint(&m_data, m_end);
            return *this;
        }

        const_varint_iterator operator++(int) {
            const const_varint_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        bool operator==(const const_varint_iterator& rhs) const noexcept {
            return m_data == rhs.m_data && m_end == rhs.m_end;
        }

        bool operator!=(const const_varint_iterator& rhs) const noexcept {
            return !(*this == rhs);
        }

    }; // class const_varint_iterator

    template <typename T>
    class const_svarint_iterator : public const_varint_iterator<T> {

    public:

        const_svarint_iterator() noexcept :
            const_varint_iterator<T>() {
        }

        const_svarint_iterator(const char *data, const char* end) noexcept :
            const_varint_iterator<T>(data, end) {
        }

        const_svarint_iterator(const const_svarint_iterator&) = default;
        const_svarint_iterator(const_svarint_iterator&&) = default;

        const_svarint_iterator& operator=(const const_svarint_iterator&) = default;
        const_svarint_iterator& operator=(const_svarint_iterator&&) = default;

        ~const_svarint_iterator() = default;

        T operator*() {
            const char* d = this->m_data; // will be thrown away
            return static_cast<T>(decode_zigzag64(decode_varint(&d, this->m_end)));
        }

        const_svarint_iterator& operator++() {
            // Ignore the result, we call decode_varint() just for the
            // side-effect of updating data.
            decode_varint(&this->m_data, this->m_end);
            return *this;
        }

        const_svarint_iterator operator++(int) {
            const const_svarint_iterator tmp(*this);
            ++(*this);
            return tmp;
        }

    }; // class const_svarint_iterator

public:

    /// Forward iterator for iterating over bool (int32 varint) values.
    typedef const_varint_iterator< int32_t> const_bool_iterator;

    /// Forward iterator for iterating over enum (int32 varint) values.
    typedef const_varint_iterator< int32_t> const_enum_iterator;

    /// Forward iterator for iterating over int32 (varint) values.
    typedef const_varint_iterator< int32_t> const_int32_iterator;

    /// Forward iterator for iterating over uint32 (varint) values.
    typedef const_varint_iterator<uint32_t> const_uint32_iterator;

    /// Forward iterator for iterating over sint32 (varint) values.
    typedef const_svarint_iterator<int32_t> const_sint32_iterator;

    /// Forward iterator for iterating over int64 (varint) values.
    typedef const_varint_iterator< int64_t> const_int64_iterator;

    /// Forward iterator for iterating over uint64 (varint) values.
    typedef const_varint_iterator<uint64_t> const_uint64_iterator;

    /// Forward iterator for iterating over sint64 (varint) values.
    typedef const_svarint_iterator<int64_t> const_sint64_iterator;

    ///@{
    /**
     * @name Repeated packed field accessor functions
     */

    /**
     * Consume current "repeated packed fixed32" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed fixed32".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<const uint32_t*, const uint32_t*> get_packed_fixed32();

    /**
     * Consume current "repeated packed fixed64" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed fixed64".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<const uint64_t*, const uint64_t*> get_packed_fixed64();

    /**
     * Consume current "repeated packed sfixed32" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed sfixed32".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<const int32_t*, const int32_t*> get_packed_sfixed32();

    /**
     * Consume current "repeated packed sfixed64" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed sfixed64".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<const int64_t*, const int64_t*> get_packed_sfixed64();

    /**
     * Consume current "repeated packed bool" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed bool".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_bool_iterator, pbf::const_bool_iterator> get_packed_bool();

    /**
     * Consume current "repeated packed enum" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed enum".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_enum_iterator, pbf::const_enum_iterator> get_packed_enum();

    /**
     * Consume current "repeated packed int32" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed int32".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_int32_iterator, pbf::const_int32_iterator> get_packed_int32();

    /**
     * Consume current "repeated packed uint32" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed uint32".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_uint32_iterator, pbf::const_uint32_iterator> get_packed_uint32();

    /**
     * Consume current "repeated packed sint32" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed sint32".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_sint32_iterator, pbf::const_sint32_iterator> get_packed_sint32();

    /**
     * Consume current "repeated packed int64" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed int64".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_int64_iterator, pbf::const_int64_iterator> get_packed_int64();

    /**
     * Consume current "repeated packed uint64" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed uint64".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_uint64_iterator, pbf::const_uint64_iterator> get_packed_uint64();

    /**
     * Consume current "repeated packed sint64" field.
     *
     * @returns a pair of iterators to the beginning and one past the end of
     *          the data.
     * @pre There must be a current field (ie. next() must have returned `true`).
     * @pre The current field must be of type "repeated packed sint64".
     * @post The current field was consumed and there is no current field now.
     */
    inline std::pair<pbf::const_sint64_iterator, pbf::const_sint64_iterator> get_packed_sint64();

    ///@}

}; // class pbf

pbf::pbf(const char *data, size_t length)
    : m_data(data),
      m_end(data + length),
      m_wire_type(pbf_wire_type::unknown),
      m_tag(0) {
}

pbf::pbf(const std::string& data)
    : m_data(data.data()),
      m_end(data.data() + data.size()),
      m_wire_type(pbf_wire_type::unknown),
      m_tag(0) {
}

pbf::operator bool() const noexcept {
    return m_data < m_end;
}

bool pbf::next() {
    if (m_data < m_end) {
        auto value = get_varint<uint32_t>();
        m_tag = value >> 3;

        // tags 0 and 19000 to 19999 are not allowed as per
        // https://developers.google.com/protocol-buffers/docs/proto
        pbf_assert(((m_tag > 0 && m_tag < 19000) || (m_tag > 19999 && m_tag <= ((1 << 29) - 1))) && "tag out of range");

        m_wire_type = pbf_wire_type(value & 0x07);
// XXX do we want this check? or should it throw an exception?
//        pbf_assert((m_wire_type <=2 || m_wire_type == 5) && "illegal wire type");
        return true;
    }
    return false;
}

bool pbf::next(pbf_tag_type requested_tag) {
    while (next()) {
        if (m_tag == requested_tag) {
            return true;
        } else {
            skip();
        }
    }
    return false;
}

pbf_tag_type pbf::tag() const noexcept {
    return m_tag;
}

pbf_wire_type pbf::wire_type() const noexcept {
    return m_wire_type;
}

bool pbf::has_wire_type(pbf_wire_type type) const noexcept {
    return wire_type() == type;
}

void pbf::skip_bytes(pbf_length_type len) {
    if (m_data + len > m_end) {
        throw end_of_buffer_exception();
    }
    m_data += len;

// In debug builds reset the tag to zero so that we can detect (some)
// wrong code.
#ifndef NDEBUG
    m_tag = 0;
#endif
}

void pbf::skip() {
    pbf_assert(tag() != 0 && "call next() before calling skip()");
    switch (wire_type()) {
        case pbf_wire_type::varint:
            (void)get_uint32(); // called for the side-effect of skipping value
            break;
        case pbf_wire_type::fixed64:
            skip_bytes(8);
            break;
        case pbf_wire_type::length_delimited:
            skip_bytes(get_length());
            break;
        case pbf_wire_type::fixed32:
            skip_bytes(4);
            break;
        default:
            throw unknown_field_type_exception();
    }
}

pbf_length_type pbf::get_len_and_skip() {
    auto len = get_length();
    skip_bytes(len);
    return len;
}

inline int32_t pbf::decode_zigzag32(uint32_t value) noexcept {
    return int32_t(value >> 1) ^ -int32_t(value & 1);
}

inline int64_t pbf::decode_zigzag64(uint64_t value) noexcept {
    return int64_t(value >> 1) ^ -int64_t(value & 1);
}

template <typename T>
T pbf::get_varint() {
    return static_cast<T>(decode_varint(&m_data, m_end));
}

template <typename T>
T pbf::get_svarint() {
    pbf_assert((has_wire_type(pbf_wire_type::varint) || has_wire_type(pbf_wire_type::length_delimited)) && "not a varint");
    return static_cast<T>(decode_zigzag64(decode_varint(&m_data, m_end)));
}

template <typename T>
T pbf::get_fixed() {
    skip_bytes(sizeof(T));
    T result;
    memcpy(&result, m_data - sizeof(T), sizeof(T));
    return result;
}

uint32_t pbf::get_fixed32() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed32) && "not a 32-bit fixed");
    return get_fixed<uint32_t>();
}

int32_t pbf::get_sfixed32() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed32) && "not a 32-bit fixed");
    return get_fixed<int32_t>();
}

uint64_t pbf::get_fixed64() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed64) && "not a 64-bit fixed");
    return get_fixed<uint64_t>();
}

int64_t pbf::get_sfixed64() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed64) && "not a 64-bit fixed");
    return get_fixed<int64_t>();
}

float pbf::get_float() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed32) && "not a 32-bit fixed");
    return get_fixed<float>();
}

double pbf::get_double() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::fixed64) && "not a 64-bit fixed");
    return get_fixed<double>();
}

bool pbf::get_bool() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::varint) && "not a varint");
    pbf_assert((*m_data & 0x80) == 0 && "not a 1 byte varint");
    skip_bytes(1);
    return m_data[-1] != 0; // -1 okay because we incremented m_data the line before
}

std::pair<const char*, pbf_length_type> pbf::get_data() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    pbf_assert(has_wire_type(pbf_wire_type::length_delimited) && "not of type string, bytes or message");
    auto len = get_len_and_skip();
    return std::make_pair(m_data-len, len);
}

std::string pbf::get_bytes() {
    auto d = get_data();
    return std::string(d.first, d.second);
}

std::string pbf::get_string() {
    return get_bytes();
}

pbf pbf::get_message() {
    auto d = get_data();
    return pbf(d.first, d.second);
}

template <typename T>
std::pair<const T*, const T*> pbf::packed_fixed() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    pbf_assert(len % sizeof(T) == 0);
    return std::make_pair(reinterpret_cast<const T*>(m_data-len), reinterpret_cast<const T*>(m_data));
}

std::pair<const uint32_t*, const uint32_t*> pbf::get_packed_fixed32() {
    return packed_fixed<uint32_t>();
}

std::pair<const uint64_t*, const uint64_t*> pbf::get_packed_fixed64() {
    return packed_fixed<uint64_t>();
}

std::pair<const int32_t*, const int32_t*> pbf::get_packed_sfixed32() {
    return packed_fixed<int32_t>();
}

std::pair<const int64_t*, const int64_t*> pbf::get_packed_sfixed64() {
    return packed_fixed<int64_t>();
}

std::pair<pbf::const_bool_iterator, pbf::const_bool_iterator> pbf::get_packed_bool() {
    return get_packed_int32();
}

std::pair<pbf::const_enum_iterator, pbf::const_enum_iterator> pbf::get_packed_enum() {
    return get_packed_int32();
}

std::pair<pbf::const_int32_iterator, pbf::const_int32_iterator> pbf::get_packed_int32() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_int32_iterator(m_data-len, m_data),
                          pbf::const_int32_iterator(m_data, m_data));
}

std::pair<pbf::const_uint32_iterator, pbf::const_uint32_iterator> pbf::get_packed_uint32() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_uint32_iterator(m_data-len, m_data),
                          pbf::const_uint32_iterator(m_data, m_data));
}

std::pair<pbf::const_sint32_iterator, pbf::const_sint32_iterator> pbf::get_packed_sint32() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_sint32_iterator(m_data-len, m_data),
                          pbf::const_sint32_iterator(m_data, m_data));
}

std::pair<pbf::const_int64_iterator, pbf::const_int64_iterator> pbf::get_packed_int64() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_int64_iterator(m_data-len, m_data),
                          pbf::const_int64_iterator(m_data, m_data));
}

std::pair<pbf::const_uint64_iterator, pbf::const_uint64_iterator> pbf::get_packed_uint64() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_uint64_iterator(m_data-len, m_data),
                          pbf::const_uint64_iterator(m_data, m_data));
}

std::pair<pbf::const_sint64_iterator, pbf::const_sint64_iterator> pbf::get_packed_sint64() {
    pbf_assert(tag() != 0 && "call next() before accessing field value");
    auto len = get_len_and_skip();
    return std::make_pair(pbf::const_sint64_iterator(m_data-len, m_data),
                          pbf::const_sint64_iterator(m_data, m_data));
}

}} // end namespace mapbox::util

#endif // MAPBOX_UTIL_PBF_READER_HPP
