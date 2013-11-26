#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

# first inherit from env
CXX := $(CXX)
CXXFLAGS := $(CXXFLAGS) -std=c++11
LDFLAGS := $(LDFLAGS)

OS:=$(shell uname -s)
ifeq ($(OS),Darwin)
	CXXFLAGS += -stdlib=libc++
	LDFLAGS += -stdlib=libc++
endif

INCLUDE_FILES := $(shell find include -name \*.hpp | sort)

WARNINGFLAGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Winline -Wold-style-cast
#WARNINGFLAGS += -Weffc++

INSTALL_USER := root

# We use the numeric id 0 here because different systems (Linux vs. BSD)
# use different names for the "root group".
INSTALL_GROUP := 0

CPPCHECK_OPTIONS := --enable=warning,style,performance,portability,information,missingInclude

# cpp doesn't find system includes for some reason, suppress that report
CPPCHECK_OPTIONS += --suppress=missingIncludeSystem

# temp fix for http://sourceforge.net/apps/trac/cppcheck/ticket/4966
CPPCHECK_OPTIONS += --suppress=constStatement

# no need to test unit test boilerplate
CPPCHECK_OPTIONS += -USTAND_ALONE

# optional stricter checks
#CPPCHECK_OPTIONS += --inconclusive

all:

.PHONY: clean install check test indent

clean:
	rm -fr check-includes doc/html test/tests

check:
	cppcheck --std=c++11 $(CPPCHECK_OPTIONS) -I include $(INCLUDE_FILES) */*.cpp test/t/*/test_*.cpp

install: doc
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/include
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/share/doc/libosmium-dev
	install -m 644 -g $(INSTALL_GROUP) -o $(INSTALL_USER) README.md $(DESTDIR)/usr/share/doc/libosmium-dev/README.md
	cp -r include/osmium $(DESTDIR)/usr/include

# This will try to compile each include file on its own to detect missing
# #include directives. Note that if this reports [OK], it is not enough
# to be sure it will compile in production code. But if it reports an error
# we know we are missing something.
.PRECIOUS: check-includes
check-includes: $(INCLUDE_FILES)
	echo "CHECK INCLUDES REPORT:" >check-includes; \
    allok=yes; \
	for FILE in $(INCLUDE_FILES); do \
        flags=`./get_options.sh --cflags $${FILE}`; \
        eval eflags=$${flags}; \
        compile="$(CXX) $(CXXFLAGS) $(WARNINGFLAGS) -I include $${eflags} $${FILE}"; \
        echo "\n======== $${FILE}\n$${compile}" >>check-includes; \
        if `$${compile} 2>>check-includes`; then \
            echo "[OK] $${FILE}"; \
        else \
            echo "[  ] $${FILE}"; \
            allok=no; \
        fi; \
        rm -f $${FILE}.gch; \
	done; \
    if test $${allok} = "yes"; then echo "All files OK"; else echo "There were errors"; fi; \
    echo "\nDONE" >>check-includes

test:
	(cd test && ./run_tests.sh)

iwyu:
	for FILE in $(INCLUDE_FILES); do \
	    echo "\n=== $${FILE} ========================================================="; \
	    iwyu --std=c++11 $${FILE}; \
	done;

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp test/\*.cpp
#	astyle --style=java --indent-namespaces --indent-switches --pad-header --unpad-paren --align-pointer=type --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp test/\*.cpp

doc: doc/html/files.html
	doxygen doc/Doxyfile >/dev/null

doc/html/files.html: $(INCLUDE_FILES) doc/Doxyfile doc/doc.txt doc/osmium.css

install-doc: doc
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/share/doc/libosmium-dev
	cp -r doc/html $(DESTDIR)/usr/share/doc/libosmium-dev

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

