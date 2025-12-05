build_gen:
	make -C swsrc/gen/

test_factor_qs:
	make -C swtest/algs test_factor_qs

test_factor_ecm:
	make -C swtest/algs test_factor_ecm
