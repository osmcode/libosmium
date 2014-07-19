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

INCLUDE_FILES := $(shell find include/osmium -name \*.hpp | sort)
IWYU_REPORT_FILES := $(subst include/osmium,check_reports,$(INCLUDE_FILES:.hpp=.iwyu))
INCLUDES_REPORT_FILES := $(subst include/osmium,check_reports,$(INCLUDE_FILES:.hpp=.compile))

WARNINGFLAGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Winline -Wold-style-cast
#WARNINGFLAGS += -Weffc++

ifeq ($(CXX),clang++)
    WARNINGFLAGS += -Wdocumentation
endif

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

.PHONY: clean install check test indent doc/includes.txt

clean:
	rm -fr includes.log iwyu.log check_reports
	rm -fr doc/html doc/xml doc/classes.txt doc/structs.txt doc/template-classes.txt doc/includes.txt test/tests tests/test_*.o
	rm -fr test/tests tests/test_*.o
	$(MAKE) -C test/osm-testdata clean

check:
	cppcheck --std=c++11 $(CPPCHECK_OPTIONS) -I include $(INCLUDE_FILES) */*.cpp test/t/*/test_*.cpp test/osm-testdata/*.cpp

install: doc
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/include
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/share/doc/libosmium-dev
	install -m 644 -g $(INSTALL_GROUP) -o $(INSTALL_USER) README.md $(DESTDIR)/usr/share/doc/libosmium-dev/README.md
	cp -r include/osmium $(DESTDIR)/usr/include

test:
	(cd test && ./run_tests.sh)
	if test -d ../osm-testdata; then $(MAKE) -C test/osm-testdata test; fi

# This will try to compile include files on their own to detect missing
# include directives and other dependency-related problems. Note that if this
# reports OK, it is not enough to be sure it will compile in production code.
# But if it reports an error we know we are missing something.
check_reports/%.compile: include/osmium/%.hpp
	@flags=`./get_options.sh --cflags $<`; \
	mkdir -p `dirname $@`; \
	echo "$<\n===========================================" >$@; \
    cmdline="$(CXX) $(CXXFLAGS) -Werror $(WARNINGFLAGS) -I include $${flags} $<"; \
	echo "$${cmdline}\n===========================================" >>$@; \
	if $${cmdline} >>$@ 2>&1; then \
        echo "\n===========================================\nOK" >>$@; \
    else \
        echo "\n===========================================\nERR" >>$@; \
    fi; \
    rm -f $<.gch;

# This will run IWYU (Include What You Use) on includes files. The iwyu
# program isn't very reliable and crashes often, but is still useful.
check_reports/%.iwyu: include/osmium/%.hpp
	@flags=`./get_options.sh --cflags $<`; \
	mkdir -p `dirname $@`; \
	echo "$<\n===========================================" >$@; \
    cmdline="iwyu -Xiwyu --mapping_file=osmium.imp $(CXXFLAGS) $(WARNINGFLAGS) -I include $${flags} $<"; \
	echo "$${cmdline}\n===========================================" >>$@; \
	$${cmdline} >>$@ 2>&1; \
	true

.PHONY: iwyu_reports iwyu includes_reports check-includes

iwyu_reports: $(IWYU_REPORT_FILES)

includes_reports: $(INCLUDES_REPORT_FILES)

iwyu: iwyu_reports
	@echo "INCLUDE WHAT YOU USE REPORT:" >iwyu.log; \
    allok=yes; \
	for FILE in $(IWYU_REPORT_FILES); do \
	    INCL=`echo $${FILE%.iwyu}.hpp | sed -e 's@check_reports@include/osmium@'`; \
        if grep -q 'has correct #includes/fwd-decls' $${FILE}; then \
	        echo "\n\033[1m\033[32m========\033[0m \033[1m$${INCL}\033[0m" >>iwyu.log; \
            echo "[OK] $${INCL}"; \
        elif grep -q 'Assertion failed' $${FILE}; then \
	        echo "\n\033[1m======== $${INCL}\033[0m" >>iwyu.log; \
            echo "[--] $${INCL}"; \
            allok=no; \
        else \
	        echo "\n\033[1m\033[31m========\033[0m \033[1m$${INCL}\033[0m" >>iwyu.log; \
            echo "[  ] $${INCL}"; \
            allok=no; \
        fi; \
	    cat $${FILE} >>iwyu.log; \
	done; \
    if test $${allok} = "yes"; then echo "All files OK"; else echo "There were errors"; fi;

check-includes: includes_reports
	@echo "INCLUDES REPORT:" >includes.log; \
    allok=yes; \
	for FILE in $(INCLUDES_REPORT_FILES); do \
	    INCL=`echo $${FILE%.compile}.hpp | sed -e 's@check_reports@include/osmium@'`; \
        if tail -1 $${FILE} | grep OK >/dev/null; then \
	        echo "\n\033[1m\033[32m========\033[0m \033[1m$${INCL}\033[0m" >>includes.log; \
            echo "[OK] $${INCL}"; \
        else \
	        echo "\n\033[1m\033[31m========\033[0m \033[1m$${INCL}\033[0m" >>includes.log; \
            echo "[  ] $${INCL}"; \
            allok=no; \
        fi; \
	    cat $${FILE} >>includes.log; \
	done; \
    if test $${allok} = "yes"; then echo "All files OK"; else echo "There were errors"; fi;

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp test/\*.cpp
#	astyle --style=java --indent-namespaces --indent-switches --pad-header --unpad-paren --align-pointer=type --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp test/\*.cpp

doc: doc/html/files.html doc/classes.txt doc/structs.txt doc/template-classes.txt doc/includes.txt

doc/html/files.html: $(INCLUDE_FILES) doc/Doxyfile doc/doc.txt doc/osmium.css
	doxygen doc/Doxyfile >/dev/null

doc/classes.txt: doc/xml/classosmium_1_1*
	xmlstarlet sel -t -i "/doxygen/compounddef/@prot='public'" -i "not(/doxygen/compounddef/templateparamlist)" -v "/doxygen/compounddef/compoundname/text()" -n doc/xml/classosmium_1_1* >$@

doc/structs.txt: doc/xml/structosmium_1_1*
	xmlstarlet sel -t -i "/doxygen/compounddef/@prot='public'" -i "not(/doxygen/compounddef/templateparamlist)" -v "/doxygen/compounddef/compoundname/text()" -n doc/xml/structosmium_1_1* >$@

doc/template-classes.txt: doc/xml/classosmium_1_1*
	xmlstarlet sel -t -i "/doxygen/compounddef/@prot='public'" -i "/doxygen/compounddef/templateparamlist" -v "/doxygen/compounddef/compoundname/text()" -n doc/xml/classosmium_1_1* >$@

doc/includes.txt:
	find include/osmium -type f -name \*.hpp | sort | cut -d/ -f2- >$@

install-doc: doc
	install -m 755 -g $(INSTALL_GROUP) -o $(INSTALL_USER) -d $(DESTDIR)/usr/share/doc/libosmium-dev
	cp -r doc/html $(DESTDIR)/usr/share/doc/libosmium-dev

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

