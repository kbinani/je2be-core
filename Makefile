.PHONY: docker_image
docker_image:
	docker build -t je2be-clang-format .

.PHONY: format
format:
	bash tool/format

.PHONY: ls-files
ls-files:
	bash tool/ls-files
