CXX = g++
RFLAGS = -c -O2 -Wall -Werror -Wextra
DFLAGS = -c -g  -Wall -Werror -Wextra
INCLUDE = -I../include/
LIBS = -lgmp -lgmpxx
OBJECTS_DIR = ../../objects/algs

factor_QS:
	$(CXX) $(RFLAGS) $(INCLUDE) $(LIBS) factor_qs.cpp \
		-o $(OBJECTS_DIR)/factor_qs.o

factor_QS_deb:
	$(CXX) $(DFLAGS) $(INCLUDE) $(LIBS) factor_qs.cpp \
		-o $(OBJECTS_DIR)/factor_qs.o

factor_ECM:
	$(CXX) $(RFLAGS) $(INCLUDE) $(LIBS) factor_ecm.cpp \
		-o $(OBJECTS_DIR)/factor_ecm.o

factor_ECM_deb:
	$(CXX) $(DFLAGS) $(INCLUDE) $(LIBS) factor_ecm.cpp \
		-o $(OBJECTS_DIR)/factor_ecm.o

all: factor_QS factor_ECM
	# pass
