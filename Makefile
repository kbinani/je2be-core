fmt:
	find include/je2be -name '*.hpp' | xargs -n 1 -P `nproc` clang-format -i

f:
	((git diff --name-status --staged; git diff --name-status) | grep '^M' | awk '{ print $$2 }'; (git diff --name-status; git diff --name-status --staged) | grep '^R' | awk '{ print $$3 }') | sort | uniq | grep include/je2be/ | grep -e '\.hpp$$' | xargs -n 1 -P `nproc` clang-format -i

