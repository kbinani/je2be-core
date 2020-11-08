fmt:
	find include/je2be -name '*.hpp' | xargs -n 1 -P `nproc` clang-format -i
