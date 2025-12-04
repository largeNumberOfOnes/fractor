CXX = g++
RFLAGS = -O2 -Wall -Werror -Wextra
DFLAGS = -g  -Wall -Werror -Wextra
INCLUDE = -I../../swsrc/include/
LIBS = -lgmp -lgmpxx
OBJECTS = ../../objects/algs/*
BUILD_DIR = ../../build/algs

test_factor_qs:
	make -C ../../swsrc/algs factor_QS_deb
	$(CXX) $(DFLAGS) $(INCLUDE) $(LIBS) factor_qs.test.cpp $(OBJECTS) \
		-o $(BUILD_DIR)/factor_qs.test.out
	./$(BUILD_DIR)/factor_qs.test.out

test_factor_ecm:
	make -C ../../swsrc/algs factor_ECM_deb
	$(CXX) $(DFLAGS) $(INCLUDE) $(LIBS) factor_ecm.test.cpp $(OBJECTS) \
		-o $(BUILD_DIR)/factor_ecm.test.out
	./$(BUILD_DIR)/factor_ecm.test.out
