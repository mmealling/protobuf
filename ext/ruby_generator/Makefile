all: \
	RubyGenerator.h \
	RubyGenerator.cpp
	g++ -g -Wall -I/code/src/utilities/protobuf-2.4.1/src -lprotoc -lprotobuf -lpthread -o protoc-ruby RubyGenerator.cpp

test:
	rm -rf test/out/*
	./protoc-ruby --proto_path=/code/src/gems/protobuf/spec/support --ruby_out=/code/src/gems/protobuf/spec/support /code/src/gems/protobuf/spec/support/**/*.proto

debug:
	make all
	gdb protoc-ruby -write -x gdb.run -s protoc-ruby.dSYM

.PHONY: all test
