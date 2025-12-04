CXX = g++
RFLAGS = -O2 -Wall -Werror -Wextra
DFLAGS = -g  -Wall -Werror -Wextra
INCLUDE = -I../../swsrc/include/
LIBS = -lgmp -lgmpxx
OBJECTS = ../../objects/algs/*
BUILD_DIR = ../../build/algs

test:
	make -C ../../swsrc/algs factor_QS_deb
	$(CXX) $(DFLAGS) $(INCLUDE) $(LIBS) factor_qs.test.cpp $(OBJECTS) \
		-o $(BUILD_DIR)/factor_qs.test.out
	./$(BUILD_DIR)/factor_qs.test.out
