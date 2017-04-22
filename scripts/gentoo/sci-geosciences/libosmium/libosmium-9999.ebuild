# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI="5"

inherit cmake-utils git-2

DESCRIPTION="Fast and flexible C++ library for working with OpenStreetMap data."
HOMEPAGE="https://github.com/osmcode/libosmium"
EGIT_REPO_URI="https://github.com/osmcode/libosmium.git"

LICENSE="Boost-1.0"
SLOT="0/9999"
KEYWORDS=""

IUSE="gdalcpp protozero utfcpp doc test"

RDEPEND="
    sys-libs/zlib
    dev-libs/expat
    app-arch/bzip2
    sci-libs/geos
    sci-libs/gdal[geos,threads]
    sci-libs/proj
    dev-cpp/sparsehash
    dev-libs/boost:=[threads]
    doc? ( app-doc/doxygen )
"
DEPEND="${RDEPEND}"

src_unpack() {
    git-2_src_unpack
}

src_configure() {
    local mycmakeargs=(
        $(cmake-utils_use gdalcpp   INSTALL_GDALCPP)
        $(cmake-utils_use protozero INSTALL_PROTOZERO)
        $(cmake-utils_use utfcpp    INSTALL_UTFCPP)

        $(cmake-utils_use doc  BUILD_DOC)
        $(cmake-utils_use test BUILD_TESTING)
    )
    cmake-utils_src_configure
}

src_test() {
    cmake-utils_src_test
}

src_install() {
    cmake-utils_src_install
}
