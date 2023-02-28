set -ue

(
	cd "$(dirname "$0")/.."
	bash tool/ls-files | grep -e '^src/' -e '^include/' | cppcheck --file-list=- -I ./include --std=c++20 -j $(nproc) --enable=all --suppressions-list=.cppcheck-suppress
)
