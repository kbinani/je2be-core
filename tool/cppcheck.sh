set -ue

(
	cd "$(dirname "$0")/.."
	cppcheck ./src ./include -Iinclude --enable=all --suppressions-list=.cppcheck-suppress --quiet
)
