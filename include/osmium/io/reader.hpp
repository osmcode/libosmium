#ifndef OSMIUM_IO_READER_HPP
#define OSMIUM_IO_READER_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2021 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <osmium/io/compression.hpp>
#include <osmium/io/detail/input_format.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/detail/read_thread.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/error.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/thread/pool.hpp>
#include <osmium/thread/util.hpp>
#include <osmium/util/config.hpp>

#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <future>
#include <memory>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

#ifndef _WIN32
# include <sys/wait.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif

namespace osmium {

    namespace io {

        namespace detail {

            inline std::size_t get_input_queue_size() noexcept {
                return osmium::config::get_max_queue_size("INPUT", 20);
            }

            inline std::size_t get_osmdata_queue_size() noexcept {
                return osmium::config::get_max_queue_size("OSMDATA", 20);
            }

        } // namespace detail

        /**
         * This is the user-facing interface for reading OSM files. Instantiate
         * an object of this class with a file name or osmium::io::File object
         * and then call read() on it in a loop until it returns an invalid
         * Buffer.
         */
        class Reader {

            // The Reader::read() function reads from a queue of buffers which
            // can contain nested buffers. These nested buffers will be in
            // here, because read() can only return a single unnested buffer.
            osmium::memory::Buffer m_back_buffers{};

            osmium::io::File m_file;

            osmium::thread::Pool* m_pool = nullptr;

            std::atomic<std::size_t> m_offset{0};

            detail::ParserFactory::create_parser_type m_creator;

            enum class status {
                okay   = 0, // normal reading
                error  = 1, // some error occurred while reading
                closed = 2, // close() called
                eof    = 3  // eof of file was reached without error
            } m_status = status::okay;

            int m_childpid = 0;

            detail::future_string_queue_type m_input_queue;

            int m_fd = -1;

            std::size_t m_file_size = 0;

            std::unique_ptr<osmium::io::Decompressor> m_decompressor;

            osmium::io::detail::ReadThreadManager m_read_thread_manager;

            detail::future_buffer_queue_type m_osmdata_queue;
            detail::queue_wrapper<osmium::memory::Buffer> m_osmdata_queue_wrapper;

            std::future<osmium::io::Header> m_header_future{};
            osmium::io::Header m_header{};

            osmium::thread::thread_handler m_thread{};

            osmium::osm_entity_bits::type m_read_which_entities = osmium::osm_entity_bits::all;
            osmium::io::read_meta m_read_metadata = osmium::io::read_meta::yes;
            osmium::io::buffers_type m_buffers_kind = osmium::io::buffers_type::any;

            void set_option(osmium::thread::Pool& pool) noexcept {
                m_pool = &pool;
            }

            void set_option(osmium::osm_entity_bits::type value) noexcept {
                m_read_which_entities = value;
            }

            void set_option(osmium::io::read_meta value) noexcept {
                // Ignore this setting if we have a history/change file,
                // because if this is set to "no", we don't see the difference
                // between visible and deleted objects.
                if (!m_file.has_multiple_object_versions()) {
                    m_read_metadata = value;
                }
            }

            void set_option(osmium::io::buffers_type value) noexcept {
                m_buffers_kind = value;
            }

            // This function will run in a separate thread.
            static void parser_thread(osmium::thread::Pool& pool,
                                      int fd,
                                      const detail::ParserFactory::create_parser_type& creator,
                                      detail::future_string_queue_type& input_queue,
                                      detail::future_buffer_queue_type& osmdata_queue,
                                      std::promise<osmium::io::Header>&& header_promise,
                                      std::atomic<std::size_t>* offset_ptr,
                                      osmium::osm_entity_bits::type read_which_entities,
                                      osmium::io::read_meta read_metadata,
                                      osmium::io::buffers_type buffers_kind,
                                      bool want_buffered_pages_removed) {
                std::promise<osmium::io::Header> promise{std::move(header_promise)};
                osmium::io::detail::parser_arguments args = {
                    pool,
                    fd,
                    input_queue,
                    osmdata_queue,
                    promise,
                    offset_ptr,
                    read_which_entities,
                    read_metadata,
                    buffers_kind,
                    want_buffered_pages_removed
                };
                creator(args)->parse();
            }

#ifndef _WIN32
            /**
             * Fork and execute the given command in the child.
             * A pipe is created between the child and the parent.
             * The child writes to the pipe, the parent reads from it.
             * This function never returns in the child.
             *
             * @param command Command to execute in the child.
             * @param filename Filename to give to command as argument.
             * @returns File descriptor of pipe in the parent.
             * @throws std::system_error if a system call fails.
             */
            static int execute(const std::string& command, const std::string& filename, int* childpid) {
                int pipefd[2];
                if (pipe(pipefd) < 0) {
                    throw std::system_error{errno, std::system_category(), "opening pipe failed"};
                }
                const pid_t pid = fork();
                if (pid < 0) {
                    throw std::system_error{errno, std::system_category(), "fork failed"};
                }
                if (pid == 0) { // child
                    // close all file descriptors except one end of the pipe
                    for (int i = 0; i < 32; ++i) {
                        if (i != pipefd[1]) {
                            ::close(i);
                        }
                    }
                    if (dup2(pipefd[1], 1) < 0) { // put end of pipe as stdout/stdin
                        std::exit(1); // NOLINT(concurrency-mt-unsafe)
                    }

                    ::open("/dev/null", O_RDONLY); // stdin
                    ::open("/dev/null", O_WRONLY); // stderr
                    // hack: -g switches off globbing in curl which allows [] to be used in file names
                    // this is important for XAPI URLs
                    // in theory this execute() function could be used for other commands, but it is
                    // only used for curl at the moment, so this is okay.
                    if (::execlp(command.c_str(), command.c_str(), "-g", filename.c_str(), nullptr) < 0) {
                        std::exit(1); // NOLINT(concurrency-mt-unsafe)
                    }
                }
                // parent
                *childpid = pid;
                ::close(pipefd[1]);
                return pipefd[0];
            }
#endif

            /**
             * Open File for reading. Handles URLs or normal files. URLs
             * are opened by executing the "curl" program (which must be installed)
             * and reading from its output.
             *
             * @returns File descriptor of open file or pipe.
             * @throws std::system_error if a system call fails.
             */
            static int open_input_file_or_url(const std::string& filename, int* childpid) {
                const std::string protocol{filename.substr(0, filename.find_first_of(':'))};
                if (protocol == "http" || protocol == "https" || protocol == "ftp" || protocol == "file") {
#ifndef _WIN32
                    return execute("curl", filename, childpid);
#else
                    throw io_error{"Reading OSM files from the network currently not supported on Windows."};
#endif
                }
                const int fd = osmium::io::detail::open_for_reading(filename);
#if __linux__
                if (fd >= 0) {
                    // Tell the kernel we are going to read this file sequentially
                    ::posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
                }
#endif
                return fd;
            }

            static std::unique_ptr<Decompressor> make_decompressor(const osmium::io::File& file, int fd, std::atomic<std::size_t>* offset_ptr) {
                const auto& factory = osmium::io::CompressionFactory::instance();
                std::unique_ptr<Decompressor> decompressor;

                if (file.buffer()) {
                    decompressor = factory.create_decompressor(file.compression(), file.buffer(), file.buffer_size());
                } else if (file.format() == file_format::pbf) {
                    decompressor = std::unique_ptr<Decompressor>{new DummyDecompressor{}};
                } else {
                    decompressor = factory.create_decompressor(file.compression(), fd);
                }

                decompressor->set_offset_ptr(offset_ptr);
                return decompressor;
            }

        public:

            /**
             * Create new Reader object.
             *
             * @param file The file (contains name and format info) to open.
             * @param args All further arguments are optional and can appear
             *             in any order:
             *
             * * osmium::osm_entities::bits: Which OSM entities (nodes, ways,
             *      relations, and/or changesets) should be read from the
             *      input file. It can speed the read up significantly if
             *      objects that are not needed anyway are not parsed.
             *
             * * osmium::io::read_meta: Read meta data or not. The default is
             *      osmium::io::read_meta::yes which means that meta data
             *      is read normally. If you set this to
             *      osmium::io::read_meta::no, meta data (like version, uid,
             *      etc.) is not read possibly speeding up the read. Not all
             *      file formats use this setting. Do *not* set this to
             *      osmium::io::read_meta::no for history or change files
             *      because you will loose the information whether an object
             *      is visible!
             *
             * * osmium::io::buffers_type: Fill entities into buffers until
             *      the buffers are full (osmium::io::buffers_type::any) or
             *      only fill entities of the same type into a buffer
             *      (osmium::io::buffers_type::single). Every time a new
             *      entity type is seen a new buffer will be started. Do not
             *      use in "single" mode if the input file is not sorted by
             *      type, otherwise this will be rather inefficient.
             *
             * * osmium::thread::Pool&: Reference to a thread pool that should
             *      be used for reading instead of the default pool. Usually
             *      it is okay to use the statically initialized shared
             *      default pool, but sometimes you want or need your own.
             *      For instance when your program will fork, using the
             *      statically initialized pool will not work.
             *
             * @throws osmium::io_error If there was an error.
             * @throws std::system_error If the file could not be opened.
             */
            template <typename... TArgs>
            explicit Reader(const osmium::io::File& file, TArgs&&... args) :
                m_file(file.check()),
                m_creator(detail::ParserFactory::instance().get_creator_function(m_file)),
                m_input_queue(detail::get_input_queue_size(), "raw_input"),
                m_fd(m_file.buffer() ? -1 : open_input_file_or_url(m_file.filename(), &m_childpid)),
                m_file_size(m_fd > 2 ? osmium::file_size(m_fd) : 0),
                m_decompressor(make_decompressor(m_file, m_fd, &m_offset)),
                m_read_thread_manager(*m_decompressor, m_input_queue),
                m_osmdata_queue(detail::get_osmdata_queue_size(), "parser_results"),
                m_osmdata_queue_wrapper(m_osmdata_queue) {

                (void)std::initializer_list<int>{
                    (set_option(args), 0)...
                };

                if (!m_pool) {
                    m_pool = &thread::Pool::default_instance();
                }

                std::promise<osmium::io::Header> header_promise;
                m_header_future = header_promise.get_future();

                const auto cpc = osmium::config::clean_page_cache_after_read();
                if (cpc >= 0) {
                    m_decompressor->set_want_buffered_pages_removed(true);
                }

                const int fd_for_parser = m_decompressor->is_real() ? -1 : m_fd;
                m_thread = osmium::thread::thread_handler{parser_thread, std::ref(*m_pool), fd_for_parser, std::ref(m_creator),
                                                          std::ref(m_input_queue), std::ref(m_osmdata_queue),
                                                          std::move(header_promise), &m_offset, m_read_which_entities,
                                                          m_read_metadata, m_buffers_kind,
                                                          m_decompressor->want_buffered_pages_removed()};
            }

            template <typename... TArgs>
            explicit Reader(const std::string& filename, TArgs&&... args) :
                Reader(osmium::io::File(filename), std::forward<TArgs>(args)...) {
            }

            template <typename... TArgs>
            explicit Reader(const char* filename, TArgs&&... args) :
                Reader(osmium::io::File(filename), std::forward<TArgs>(args)...) {
            }

            Reader(const Reader&) = delete;
            Reader& operator=(const Reader&) = delete;

            Reader(Reader&&) = delete;
            Reader& operator=(Reader&&) = delete;

            ~Reader() noexcept {
                try {
                    close();
                } catch (...) {
                    // Ignore any exceptions because destructor must not throw.
                }
            }

            /**
             * Close down the Reader. A call to this is optional, because the
             * destructor of Reader will also call this. But if you don't call
             * this function first, you might miss an exception, because the
             * destructor is not allowed to throw.
             *
             * @throws Some form of osmium::io_error when there is a problem.
             */
            void close() {
                m_status = status::closed;

                m_read_thread_manager.stop();

                m_osmdata_queue_wrapper.drain();

                try {
                    m_read_thread_manager.close();
                } catch (...) {
                    // Ignore any exceptions.
                }

#ifndef _WIN32
                if (m_childpid) {
                    int status = 0;
                    const pid_t pid = ::waitpid(m_childpid, &status, 0);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
                    if (pid < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) { // NOLINT(hicpp-signed-bitwise)
                        throw std::system_error{errno, std::system_category(), "subprocess returned error"};
                    }
#pragma GCC diagnostic pop
                    m_childpid = 0;
                }
#endif
            }

            /**
             * Get the header data from the file.
             *
             * @returns Header.
             * @throws Some form of osmium::io_error if there is an error.
             */
            osmium::io::Header header() {
                if (m_status == status::error) {
                    throw io_error{"Can not get header from reader when in status 'error'"};
                }

                try {
                    if (m_header_future.valid()) {
                        m_header = m_header_future.get();
                    }
                } catch (...) {
                    close();
                    m_status = status::error;
                    throw;
                }

                return m_header;
            }

            /**
             * Reads the next buffer from the input. An invalid buffer signals
             * end-of-file. After end-of-file all read() calls will throw an
             * osmium::io_error.
             *
             * @returns Buffer.
             * @throws Some form of osmium::io_error if there is an error.
             */
            osmium::memory::Buffer read() {
                osmium::memory::Buffer buffer;

                // If there are buffers on the stack, return those first.
                if (m_back_buffers) {
                    if (m_back_buffers.has_nested_buffers()) {
                        buffer = std::move(*m_back_buffers.get_last_nested());
                    } else {
                        buffer = std::move(m_back_buffers);
                        m_back_buffers = osmium::memory::Buffer{};
                    }
                    return buffer;
                }

                if (m_status != status::okay) {
                    throw io_error{"Can not read from reader when in status 'closed', 'eof', or 'error'"};
                }

                if (m_read_which_entities == osmium::osm_entity_bits::nothing) {
                    m_status = status::eof;
                    return buffer;
                }

                try {
                    // m_input_format.read() can return an invalid buffer to signal EOF,
                    // or a valid buffer with or without data. A valid buffer
                    // without data is not an error, it just means we have to
                    // keep getting the next buffer until there is one with data.
                    while (true) {
                        buffer = m_osmdata_queue_wrapper.pop();
                        if (detail::at_end_of_data(buffer)) {
                            m_status = status::eof;
                            m_read_thread_manager.close();
                            return buffer;
                        }
                        if (buffer.has_nested_buffers()) {
                            m_back_buffers = std::move(buffer);
                            buffer = std::move(*m_back_buffers.get_last_nested());
                        }
                        if (buffer.committed() > 0) {
                            return buffer;
                        }
                    }
                } catch (...) {
                    close();
                    m_status = status::error;
                    throw;
                }
            }

            /**
             * Has the end of file been reached? This is set after the last
             * data has been read. It is also set by calling close().
             */
            bool eof() const {
                return m_status == status::eof || m_status == status::closed;
            }

            /**
             * Get the size of the input file. Returns 0 if the file size
             * is not available (for instance when reading from stdin).
             */
            std::size_t file_size() const noexcept {
                return m_file_size;
            }

            /**
             * Returns the current offset into the input file. Returns 0 if
             * the offset is not available (for instance when reading from
             * stdin).
             *
             * The offset can be used together with the result of file_size()
             * to estimate how much of the file has been read. Note that due
             * to buffering inside Osmium, this value will be larger than
             * the amount of data actually available to the application.
             *
             * Do not call this function too often, certainly not for every
             * object you are reading. Depending on the file type it might
             * do an expensive system call.
             */
            std::size_t offset() const noexcept {
                return m_offset;
            }

        }; // class Reader

        /**
         * Read contents of the given file into a buffer in one go. Takes
         * the same arguments as any of the Reader constructors.
         *
         * The buffer can take up quite a lot of memory, so don't do this
         * unless you are working with small OSM files and/or have lots of
         * RAM.
         */
        template <typename... TArgs>
        osmium::memory::Buffer read_file(TArgs&&... args) {
            osmium::memory::Buffer buffer{1024UL * 1024UL, osmium::memory::Buffer::auto_grow::yes};

            Reader reader{std::forward<TArgs>(args)...};
            while (auto read_buffer = reader.read()) {
                buffer.add_buffer(read_buffer);
                buffer.commit();
            }

            return buffer;
        }

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_READER_HPP
