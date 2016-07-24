CPPFLAGS=-I src/eigen -Wall -std=c++14 -O3 -msse2

all:	bin/test bin/octopi

bin/octopi:	bin/model_const.o bin/model_adaptive.o bin/encoder.o src/oct.cpp bin/decoder.o src/compression/bytesbits.h bin/model_gru.o bin/model_rnn2.o
	g++ $(CPPFLAGS) bin/model_adaptive.o bin/model_const.o bin/encoder.o bin/decoder.o bin/model_gru.o bin/model_rnn2.o src/oct.cpp -o bin/octopi

bin/test:	bin/model_const.o bin/model_adaptive.o bin/encoder.o src/test.cpp bin/decoder.o src/compression/bytesbits.h bin/model_gru.o bin/model_rnn2.o
	g++ $(CPPFLAGS) bin/model_adaptive.o bin/model_const.o bin/encoder.o bin/decoder.o bin/model_gru.o bin/model_rnn2.o src/test.cpp -o bin/test

bin/model_const.o: src/models/model_const.cpp src/models/model_const.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/models/model_const.cpp -o ./bin/model_const.o

bin/model_adaptive.o: src/models/model_adaptive.cpp src/models/model_adaptive.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/models/model_adaptive.cpp -o ./bin/model_adaptive.o

bin/model_gru.o: src/models/model_gru.cpp src/models/model_gru.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/models/model_gru.cpp -o ./bin/model_gru.o

bin/model_rnn2.o: src/models/model_rnn2.cpp src/models/model_rnn2.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/models/model_rnn2.cpp -o ./bin/model_rnn2.o

bin/encoder.o:	src/compression/encoder.cpp src/compression/encoder.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/compression/encoder.cpp -o ./bin/encoder.o
bin/decoder.o:	src/compression/decoder.cpp src/compression/decoder.h src/model_abstract.h
	g++ $(CPPFLAGS) -c ./src/compression/decoder.cpp -o ./bin/decoder.o

clean:
	rm ./bin/*.o ./bin/test || true
