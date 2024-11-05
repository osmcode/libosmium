#ifndef OSMIUM_OSM_METADATA_OPTIONS_HPP
#define OSMIUM_OSM_METADATA_OPTIONS_HPP

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

#include <osmium/osm/object.hpp>
#include <osmium/util/string.hpp>

#include <ostream>
#include <stdexcept>
#include <string>

namespace osmium {

    /**
     * Describes which metadata fields are available in an OSMObject. This is
     * most often used to describe which metadata fields are available in the
     * objects of an OSM file or which metadata information should be written
     * to an OSM file.
     */
    class metadata_options {

        enum options : unsigned int {
            md_none      = 0x00,
            md_version   = 0x01,
            md_timestamp = 0x02,
            md_changeset = 0x04,
            md_uid       = 0x08,
            md_user      = 0x10,
            md_all       = 0x1f
        } m_options = md_all;

    public:

        metadata_options() noexcept = default;

        explicit metadata_options(const std::string& attributes) {
            if (attributes.empty() || attributes == "all" || attributes == "true" || attributes == "yes") {
                return;
            }
            if (attributes == "none" || attributes == "false" || attributes == "no") {
                m_options = options::md_none;
                return;
            }

            const auto attrs = osmium::split_string(attributes, '+', true);
            unsigned int opts = 0;
            for (const auto& attr : attrs) {
                if (attr == "version") {
                    opts |= options::md_version;
                } else if (attr == "timestamp") {
                    opts |= options::md_timestamp;
                } else if (attr == "changeset") {
                    opts |= options::md_changeset;
                } else if (attr == "uid") {
                    opts |= options::md_uid;
                } else if (attr == "user") {
                    opts |= options::md_user;
                } else {
                    throw std::invalid_argument{std::string{"Unknown OSM object metadata attribute: '"} + attr + "'"};
                }
            }
            m_options = static_cast<options>(opts);
        }

        /// At least one metadata attribute should be stored.
        bool any() const noexcept {
            return m_options != 0;
        }

        /// All metadata attributes should be stored.
        bool all() const noexcept {
            return m_options == options::md_all;
        }

        /// No metadata attributes should be stored.
        bool none() const noexcept {
            return m_options == 0;
        }

        bool version() const noexcept {
            return (m_options & options::md_version) != 0;
        }

        void set_version(bool flag) noexcept {
            if (flag) {
                m_options = static_cast<options>(m_options | options::md_version);
            } else {
                m_options = static_cast<options>(m_options & ~options::md_version);
            }
        }

        bool timestamp() const noexcept {
            return (m_options & options::md_timestamp) != 0;
        }

        void set_timestamp(bool flag) noexcept {
            if (flag) {
                m_options = static_cast<options>(m_options | options::md_timestamp);
            } else {
                m_options = static_cast<options>(m_options & ~options::md_timestamp);
            }
        }

        bool changeset() const noexcept {
            return (m_options & options::md_changeset) != 0;
        }

        void set_changeset(bool flag) noexcept {
            if (flag) {
                m_options = static_cast<options>(m_options | options::md_changeset);
            } else {
                m_options = static_cast<options>(m_options & ~options::md_changeset);
            }
        }

        bool uid() const noexcept {
            return (m_options & options::md_uid) != 0;
        }

        void set_uid(bool flag) noexcept {
            if (flag) {
                m_options = static_cast<options>(m_options | options::md_uid);
            } else {
                m_options = static_cast<options>(m_options & ~options::md_uid);
            }
        }

        bool user() const noexcept {
            return (m_options & options::md_user) != 0;
        }

        void set_user(bool flag) noexcept {
            if (flag) {
                m_options = static_cast<options>(m_options | options::md_user);
            } else {
                m_options = static_cast<options>(m_options & ~options::md_user);
            }
        }

        metadata_options operator&=(const metadata_options& other) {
            m_options = static_cast<options>(other.m_options & m_options);
            return *this;
        }

        metadata_options operator|=(const metadata_options& other) {
            m_options = static_cast<options>(other.m_options | m_options);
            return *this;
        }

        std::string to_string() const {
            std::string result;

            if (none()) {
                result = "none";
                return result;
            }

            if (all()) {
                result = "all";
                return result;
            }

            if (version()) {
                result += "version+";
            }

            if (timestamp()) {
                result += "timestamp+";
            }

            if (changeset()) {
                result += "changeset+";
            }

            if (uid()) {
                result += "uid+";
            }

            if (user()) {
                result += "user+";
            }

            // remove last '+' character
            result.pop_back();

            return result;
        }

    }; // class metadata_options

    template <typename TChar, typename TTraits>
    inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, const metadata_options& options) {
        return out << options.to_string();
    }

    /**
     * Create an instance of metadata_options based on the availability of
     * metadata of an instance of osmium::OSMObject.
     */
    inline osmium::metadata_options detect_available_metadata(const osmium::OSMObject& object) {
        osmium::metadata_options opts;

        opts.set_version(object.version() > 0);
        opts.set_timestamp(object.timestamp().valid());
        opts.set_changeset(object.changeset() > 0);

        // Objects by anonymous users don't have these attributes set. There is no way
        // to distinguish them from objects with a reduced number of metadata fields.
        opts.set_uid(object.uid() > 0);
        opts.set_user(object.user()[0] != '\0');

        return opts;
    }

} // namespace osmium

#endif // OSMIUM_OSM_METADATA_OPTIONS_HPP
