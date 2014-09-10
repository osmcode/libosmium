all: osmium.node

./node_modules/.bin/node-gyp:
	npm install node-gyp

./build: binding.gyp ./node_modules/.bin/node-gyp
	./node_modules/.bin/node-gyp configure

osmium.node: Makefile ./build
	./node_modules/.bin/node-gyp build

clean:
	rm -rf ./build
	rm -f lib/osmium.node

rebuild:
	@make clean
	@make

test: osmium.node
	@PATH="./node_modules/mocha/bin:${PATH}" && NODE_PATH="./lib:$(NODE_PATH)" mocha -R spec --timeout 10000

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none src/\*pp

check: test

.PHONY: test indent
