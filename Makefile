.PHONY: docker-image
docker-image:
	@docker build -t je2be-clang-format .

.PHONY: format
format:
	@bash tool/format
