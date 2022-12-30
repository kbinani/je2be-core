# https://pkgs.alpinelinux.org/packages?name=clang-extra-tools&branch=v3.15&repo=main&arch=x86_64&maintainer=
# https://hub.docker.com/_/alpine/tags
FROM alpine:3.15.6
RUN apk add --no-cache clang-extra-tools git bash \
	&& mkdir /workspace \
	&& echo "[safe]" > /.gitconfig \
	&& echo "	directory = /workspace" >> /.gitconfig
