dir="$1"
if [ -z "$1" ]; then
	dir="$(pwd)"
fi

set -ue

script="$(cd "$(dirname "$0")"; pwd)/git-restore-mtime-bare"

function restore { (
	local directory="$1"
	cd "$directory"
	echo "restore mtime for $(pwd)..."
	python "$script"
	git submodule status | while read line; do
		local repo="$(echo "$line" | awk '{print $2}')"
		restore "$repo"
	done
) }

restore "$dir"
