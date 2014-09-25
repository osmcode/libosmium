
CXXFLAGS := $(CXXFLAGS) -std=c++11 -I/usr/local/include/node -I../libosmium/include
CXXFLAGS_WARNINGS := -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wold-style-cast

INCLUDE_FILES := $(shell find src -name \*.hpp | sort)
IWYU_REPORT_FILES := $(subst src,check_reports,$(INCLUDE_FILES:.hpp=.iwyu))
INCLUDES_REPORT_FILES := $(subst src,check_reports,$(INCLUDE_FILES:.hpp=.compile))

DEMOS := $(shell find demo -mindepth 1 -maxdepth 1 -type d)

all: build

./node_modules:
	npm install --build-from-source

build: ./node_modules
	./node_modules/.bin/node-pre-gyp build --loglevel=silent

debug:
	./node_modules/.bin/node-pre-gyp rebuild --debug

verbose:
	./node_modules/.bin/node-pre-gyp rebuild --loglevel=verbose

clean:
	rm -rf ./build ./check_reports lib/binding
	rm -f includes.log iwyu.log
	rm -rf lib/binding/

distclean:
	rm -rf node_modules demo/*/node_modules

rebuild:
	@make clean
	@make

test: build
	@PATH="./node_modules/mocha/bin:${PATH}" && NODE_PATH="./lib:$(NODE_PATH)" mocha -R spec --timeout 10s

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none src/\*pp

check: test

# Install all dependencies and run the demos. Because the demos usually need
# one or more arguments that are not supplied, they dont't do much though.
demos:
	for demo in $(DEMOS); do \
	    cd $${demo}; \
	    npm install; \
        ./index.js; \
        cd ../..; \
    done

# This will try to compile include files on their own to detect missing
# include directives and other dependency-related problems. Note that if this
# reports OK, it is not enough to be sure it will compile in production code.
# But if it reports an error we know we are missing something.
check_reports/%.compile: src/%.hpp
	mkdir -p `dirname $@`; \
	echo "$<\n===========================================" >$@; \
    cmdline="$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -I include $<"; \
	echo "$${cmdline}\n===========================================" >>$@; \
	if $${cmdline} >>$@ 2>&1; then \
        echo "\n===========================================\nOK" >>$@; \
    else \
        echo "\n===========================================\nERR" >>$@; \
    fi; \
    rm -f $<.gch;

# This will run IWYU (Include What You Use) on includes files. The iwyu
# program isn't very reliable and crashes often, but is still useful.
check_reports/%.iwyu: src/%.hpp
	mkdir -p `dirname $@`; \
	echo "$<\n===========================================" >$@; \
    cmdline="iwyu -Xiwyu -- $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -I include $<"; \
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
	    INCL=`echo $${FILE%.iwyu}.hpp | sed -e 's@check_reports@src@'`; \
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
	    INCL=`echo $${FILE%.compile}.hpp | sed -e 's@check_reports@src@'`; \
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

.PHONY: test indent
