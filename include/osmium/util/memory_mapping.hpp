#ifndef OSMIUM_UTIL_MEMORY_MAPPING_HPP
#define OSMIUM_UTIL_MEMORY_MAPPING_HPP

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
#include <cerrno>
#include <stdexcept>
#include <system_error>

#include <osmium/util/file.hpp>

#ifndef _WIN32
# include <sys/mman.h>
#else
# include <io.h>
# include <windows.h>
# include <sys/types.h>
#endif

namespace osmium {

    namespace util {

        /**
         * Class for wrapping memory mapping system calls.
         *
         * There are different implementations for Unix and Windows systems.
         * On Unix systems this wraps the mmap(), munmap(), and the mremap()
         * system calls. On Windows it wraps the CreateFileMapping(),
         * CloseHandle(), MapViewOfFile(), and UnmapViewOfFile() functions.
         *
         * Usage for anonymous mapping:
         * @code
         * MemoryMapping mapping(1024);          // create anonymous mapping with size
         * auto ptr = mapping.get_addr<char*>(); // get pointer to memory
         * mapping.unmap();                      // release mapping by calling unmap() (or at end of scope)
         * @endcode
         *
         * Or for file-backed mapping:
         * @code
         * int fd = ::open(...);
         * {
         *     MemoryMapping mapping(1024, true, fd, offset);
         *     // use mapping
         * }
         * ::close(fd);
         * @endcode
         */
        class MemoryMapping {

            /// The size of the mapping
            size_t m_size;

            /// Offset into the file
            off_t m_offset;

            /// File handle we got the mapping from
            int m_fd;

            /// Is the memory writable?
            bool m_writable;

#ifdef _WIN32
            HANDLE m_handle;
#endif

            /// The address where the memory is mapped
            void* m_addr;

            bool is_valid() const noexcept;

            void make_invalid() noexcept;

#ifdef _WIN32
            typedef DWORD flag_type;
#else
            typedef int flag_type;
#endif

            flag_type get_protection() const noexcept;

            flag_type get_flags() const noexcept;

#ifdef _WIN32
            HANDLE get_handle() const noexcept;
            HANDLE osmium::util::MemoryMapping::create_file_mapping() const noexcept;
            void* osmium::util::MemoryMapping::map_view_of_file() const noexcept;
#endif

            int resize_fd(int fd) {
                // Anonymous mapping doesn't need resizing.
                if (fd == -1) {
                    return -1;
                }

                // Make sure the file backing this mapping is large enough.
                if (osmium::util::file_size(fd) < m_size + m_offset) {
                    osmium::util::resize_file(fd, m_size + m_offset);
                }
                return fd;
            }

        public:

            /**
             * Create file-backed memory mapping of given size. The file must
             * contain at least `size` bytes!
             *
             * @param size Size of the mapping in bytes
             * @param writable Should the mapping be writable?
             * @param fd Open file descriptor of a file we want to map
             * @param offset Offset into the file where the mapping should start
             * @throws std::system_error if the mapping fails
             */
            MemoryMapping(size_t size, bool writable=true, int fd=-1, off_t offset=0);

            /// You can not copy construct a MemoryMapping.
            MemoryMapping(const MemoryMapping&) = delete;

            /// You can not copy a MemoryMapping.
            MemoryMapping& operator=(const MemoryMapping&) = delete;

            /**
             * Move construct a mapping from another one. The other mapping
             * will be marked as invalid.
             */
            MemoryMapping(MemoryMapping&& other);

            /**
             * Move a mapping. The other mapping will be marked as invalid.
             */
            MemoryMapping& operator=(MemoryMapping&& other);

            /**
             * Releases the mapping by calling unmap(). Will never throw.
             * Call unmap() instead if you want to be notified of any error.
             */
            ~MemoryMapping() noexcept {
                try {
                    unmap();
                } catch (std::system_error&) {
                    // ignore
                }
            }

            /**
             * Unmap a mapping. If the mapping is not valid, it will do
             * nothing.
             *
             * @throws std::system_error if the unmapping fails
             */
            void unmap();

            /**
             * Resize a mapping to the given new size.
             *
             * On Linux systems this will use the mremap() function. On other
             * systems it will unmap and remap the memory. This can only be
             * done for file-based mappings, not anonymous mappings!
             *
             * @param new_size Number of bytes to resize to
             * @throws std::system_error if the remapping fails
             */
            void resize(size_t new_size);

            /**
             * In a boolean context a MemoryMapping is true when it is a valid
             * existing mapping.
             */
            operator bool() const noexcept {
                return is_valid();
            }

            /**
             * The number of bytes mapped. This is the same size you created
             * the mapping with. The actual mapping will probably be larger
             * because the system will round it to the page size.
             */
            size_t size() const noexcept {
                return m_size;
            }

            /**
             * The file descriptor this mapping was created from.
             *
             * @returns file descriptor, -1 for anonymous mappings
             */
            int fd() const noexcept {
                return m_fd;
            }

            /**
             * Was this mapping created as a writable mapping?
             */
            bool writable() const noexcept {
                return m_writable;
            }

            /**
             * Get the address of the mapping as any pointer type you like.
             *
             * @throws std::runtime_error if the mapping is invalid
             */
            template <typename T = void>
            T* get_addr() const {
                if (is_valid()) {
                    return reinterpret_cast<T*>(m_addr);
                }
                throw std::runtime_error("invalid memory mapping");
            }

        }; // class MemoryMapping

        /**
         * Anonymous memory mapping.
         *
         * Usage for anonymous mapping:
         * @code
         * AnonymousMemoryMapping mapping(1024); // create anonymous mapping with size
         * auto ptr = mapping.get_addr<char*>(); // get pointer to memory
         * mapping.unmap();                      // release mapping by calling unmap() (or at end of scope)
         * @endcode
         */
        class AnonymousMemoryMapping : public MemoryMapping {

        public:

            AnonymousMemoryMapping(size_t size) :
                MemoryMapping(size) {
            }

#ifndef __linux__
            /**
             * On systems other than Linux anonymous mappings can not be
             * resized!
             */
            void resize(size_t) = delete;
#endif

        }; // class AnonymousMemoryMapping

        /**
         * A thin wrapper around the MemoryMapping class used when all the
         * data in the mapped memory is of the same type. Instead of thinking
         * about the number of bytes mapped, this counts sizes in the number
         * of objects of that type.
         */
        template <typename T>
        class TypedMemoryMapping {

            MemoryMapping m_mapping;
            size_t m_size;

        public:

            /**
             * Create anonymous memory mapping of given size.
             *
             * @param size Number of objects of type T to be mapped
             * @throws std::system_error if the mapping fails
             */
            TypedMemoryMapping(size_t size) :
                m_mapping(sizeof(T) * size),
                m_size(size) {
            }

            /**
             * Create file-backed memory mapping of given size. The file must
             * contain at least `sizeof(T) * size` bytes!
             *
             * @param size Number of objects of type T to be mapped
             * @param writable Should the mapping be writable?
             * @param fd Open file descriptor of a file we want to map
             * @param offset Offset into the file where the mapping should start
             * @throws std::system_error if the mapping fails
             */
            TypedMemoryMapping(size_t size, bool writable, int fd, off_t offset = 0) :
                m_mapping(sizeof(T) * size, writable, fd, sizeof(T) * offset),
                m_size(size) {
            }

            /// You can not copy construct a TypedMemoryMapping.
            TypedMemoryMapping(const TypedMemoryMapping&) = delete;

            /// You can not copy a MemoryMapping.
            TypedMemoryMapping& operator=(const TypedMemoryMapping&) = delete;

            /**
             * Move construct a mapping from another one. The other mapping
             * will be marked as invalid.
             */
            TypedMemoryMapping(TypedMemoryMapping&& other) = default;

            /**
             * Move a mapping. The other mapping will be marked as invalid.
             */
            TypedMemoryMapping& operator=(TypedMemoryMapping&& other) = default;

            /**
             * Releases the mapping by calling unmap(). Will never throw.
             * Call unmap() instead if you want to be notified of any error.
             */
            ~TypedMemoryMapping() = default;

            /**
             * Unmap a mapping. If the mapping is not valid, it will do
             * nothing.
             *
             * @throws std::system_error if the unmapping fails
             */
            void unmap() {
                m_mapping.unmap();
            }

            /**
             * Resize a mapping to the given new size.
             *
             * On Linux systems this will use the mremap() function. On other
             * systems it will unmap and remap the memory. This can only be
             * done for file-based mappings, not anonymous mappings!
             *
             * @param new_size Number of objects of type T to resize to
             * @throws std::system_error if the remapping fails
             */
            void resize(size_t new_size) {
                m_mapping.resize(sizeof(T) * new_size);
            }

            /**
             * In a boolean context a TypedMemoryMapping is true when it is
             * a valid existing mapping.
             */
            operator bool() const noexcept {
                return !!m_mapping;
            }

            /**
             * The number of objects of class T mapped. This is the same size
             * you created the mapping with. The actual mapping will probably
             * be larger because the system will round it to the page size.
             */
            size_t size() const noexcept {
                return m_size;
            }

            /**
             * The file descriptor this mapping was created from.
             *
             * @returns file descriptor, -1 for anonymous mappings
             */
            int fd() const noexcept {
                return m_mapping.fd();
            }

            /**
             * Was this mapping created as a writable mapping?
             */
            bool writable() const noexcept {
                return m_mapping.writable();
            }

            /**
             * Get the address of the mapping.
             *
             * @throws std::runtime_error if the mapping is invalid
             */
            T* get_addr() const {
                return m_mapping.get_addr<T>();
            }

        }; // class TypedMemoryMapping

        template <typename T>
        class AnonymousTypedMemoryMapping : public TypedMemoryMapping<T> {

        public:

            AnonymousTypedMemoryMapping(size_t size) :
                TypedMemoryMapping<T>(size) {
            }

#ifndef __linux__
            /**
             * On systems other than Linux anonymous mappings can not be
             * resized!
             */
            void resize(size_t) = delete;
#endif

        }; // class AnonymousTypedMemoryMapping

    } // namespace util

} // namespace osmium

#ifndef _WIN32

// =========== Unix implementation =============

// MAP_FAILED is often a macro containing an old style cast
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline bool osmium::util::MemoryMapping::is_valid() const noexcept {
    return m_addr != MAP_FAILED;
}

inline void osmium::util::MemoryMapping::make_invalid() noexcept {
    m_addr = MAP_FAILED;
}

#pragma GCC diagnostic pop

// for BSD systems
#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif

inline int osmium::util::MemoryMapping::get_protection() const noexcept {
    if (m_writable) {
        return PROT_READ | PROT_WRITE;
    }
    return PROT_READ;
}

inline int osmium::util::MemoryMapping::get_flags() const noexcept {
    if (m_fd == -1) {
        return MAP_PRIVATE | MAP_ANONYMOUS;
    }
    return MAP_SHARED;
}

inline osmium::util::MemoryMapping::MemoryMapping(size_t size, bool writable, int fd, off_t offset) :
    m_size(osmium::util::round_to_pagesize(size)),
    m_offset(offset),
    m_fd(resize_fd(fd)),
    m_writable(writable),
    m_addr(::mmap(nullptr, m_size, get_protection(), get_flags(), m_fd, m_offset)) {
    assert(writable || fd != -1);
    if (!is_valid()) {
        throw std::system_error(errno, std::system_category(), "mmap failed");
    }
}

inline osmium::util::MemoryMapping::MemoryMapping(MemoryMapping&& other) :
    m_size(other.m_size),
    m_offset(other.m_offset),
    m_fd(other.m_fd),
    m_writable(other.m_writable),
    m_addr(other.m_addr) {
    other.make_invalid();
}

inline osmium::util::MemoryMapping& osmium::util::MemoryMapping::operator=(osmium::util::MemoryMapping&& other) {
    unmap();
    m_size     = other.m_size;
    m_offset   = other.m_offset;
    m_fd       = other.m_fd;
    m_writable = other.m_writable;
    m_addr     = other.m_addr;
    other.make_invalid();
    return *this;
}

inline void osmium::util::MemoryMapping::unmap() {
    if (is_valid()) {
        if (::munmap(m_addr, m_size) != 0) {
            throw std::system_error(errno, std::system_category(), "munmap failed");
        }
        make_invalid();
    }
}

inline void osmium::util::MemoryMapping::resize(size_t new_size) {
    if (m_fd == -1) { // anonymous mapping
#ifdef __linux__
        size_t old_size = m_size;
        m_size = osmium::util::round_to_pagesize(new_size);
        m_addr = ::mremap(m_addr, old_size, m_size, MREMAP_MAYMOVE);
        if (!is_valid()) {
            throw std::system_error(errno, std::system_category(), "mremap failed");
        }
#else
        assert(false && "can't resize anonymous mappings on non-linux systems");
#endif
    } else { // file-based mapping
        unmap();
        m_size = osmium::util::round_to_pagesize(new_size);
        resize_fd(m_fd);
        m_addr = ::mmap(nullptr, new_size, get_protection(), get_flags(), m_fd, m_offset);
        if (!is_valid()) {
            throw std::system_error(errno, std::system_category(), "mmap (remap) failed");
        }
    }
}

#else

// =========== Windows implementation =============

/* References:
 * CreateFileMapping: http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
 * CloseHandle:       http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
 * MapViewOfFile:     http://msdn.microsoft.com/en-us/library/aa366761(VS.85).aspx
 * UnmapViewOfFile:   http://msdn.microsoft.com/en-us/library/aa366882(VS.85).aspx
 */

namespace {

    inline DWORD dword_hi(uint64_t x) {
        return static_cast<DWORD>(x >> 32);
    }

    inline DWORD dword_lo(uint64_t x) {
        return static_cast<DWORD>(x & 0xffffffff);
    }

} // anonymous namespace

inline DWORD osmium::util::MemoryMapping::get_protection() const noexcept {
    if (m_writable) {
        return PAGE_READWRITE;
    }
    return PAGE_READONLY;
}

inline DWORD osmium::util::MemoryMapping::get_flags() const noexcept {
    if (m_fd == -1) {
        return FILE_MAP_WRITE | FILE_MAP_COPY;
    }
    if (m_writable) {
        return FILE_MAP_WRITE;
    }
    return FILE_MAP_READ;
}

inline HANDLE osmium::util::MemoryMapping::get_handle() const noexcept {
    if (m_fd == -1) {
        return INVALID_HANDLE_VALUE;
    }
    return reinterpret_cast<HANDLE>(_get_osfhandle(m_fd));
}

inline HANDLE osmium::util::MemoryMapping::create_file_mapping() const noexcept {
    return CreateFileMapping(get_handle(), nullptr, get_protection(), dword_hi(static_cast<uint64_t>(m_size) + m_offset), dword_lo(static_cast<uint64_t>(m_size) + m_offset), nullptr);
}

inline void* osmium::util::MemoryMapping::map_view_of_file() const noexcept {
    return MapViewOfFile(m_handle, get_flags(), dword_hi(m_offset), dword_lo(m_offset), m_size);
}

inline bool osmium::util::MemoryMapping::is_valid() const noexcept {
    return m_addr != nullptr;
}

inline void osmium::util::MemoryMapping::make_invalid() noexcept {
    m_addr = nullptr;
}

inline osmium::util::MemoryMapping::MemoryMapping(size_t size, bool writable, int fd, off_t offset) :
    m_size(osmium::util::round_to_pagesize(size)),
    m_offset(offset),
    m_fd(resize_fd(fd)),
    m_writable(writable),
    m_handle(create_file_mapping()),
    m_addr(nullptr) {

    if (!m_handle) {
        throw std::system_error(GetLastError(), std::system_category(), "CreateFileMapping failed");
    }

    m_addr = map_view_of_file();
    if (!is_valid()) {
        throw std::system_error(GetLastError(), std::system_category(), "MapViewOfFile failed");
    }
}

inline osmium::util::MemoryMapping::MemoryMapping(MemoryMapping&& other) :
    m_size(other.m_size),
    m_offset(other.m_offset),
    m_fd(other.m_fd),
    m_writable(other.m_writable),
    m_handle(std::move(other.m_handle)),
    m_addr(other.m_addr) {
    other.make_invalid();
    other.m_handle = nullptr;
}

inline osmium::util::MemoryMapping& osmium::util::MemoryMapping::operator=(osmium::util::MemoryMapping&& other) {
    unmap();
    m_size     = other.m_size;
    m_offset   = other.m_offset;
    m_fd       = other.m_fd;
    m_writable = other.m_writable;
    m_handle   = std::move(other.m_handle);
    m_addr     = other.m_addr;
    other.make_invalid();
    other.m_handle = nullptr;
    return *this;
}

inline void osmium::util::MemoryMapping::unmap() {
    if (is_valid()) {
        if (! UnmapViewOfFile(m_addr)) {
            throw std::system_error(GetLastError(), std::system_category(), "UnmapViewOfFile failed");
        }
        make_invalid();
    }

    if (m_handle) {
        if (! CloseHandle(m_handle)) {
            throw std::system_error(GetLastError(), std::system_category(), "CloseHandle failed");
        }
        m_handle = nullptr;
    }
}

inline void osmium::util::MemoryMapping::resize(size_t new_size) {
    unmap();

    m_size = osmium::util::round_to_pagesize(new_size);
    resize_fd(m_fd);

    m_handle = create_file_mapping();
    if (!m_handle) {
        throw std::system_error(GetLastError(), std::system_category(), "CreateFileMapping failed");
    }

    m_addr = map_view_of_file();
    if (!is_valid()) {
        throw std::system_error(GetLastError(), std::system_category(), "MapViewOfFile failed");
    }
}

#endif

#endif // OSMIUM_UTIL_MEMORY_MAPPING_HPP
