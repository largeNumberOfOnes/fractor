gen:
	make -C swsrc/gen/

fr:
	make -C swsrc/fr/

test_factor_qs:
	make -C swtest/algs test_factor_qs

test_factor_ecm:
	make -C swtest/algs test_factor_ecm
