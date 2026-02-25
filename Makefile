# Root Makefile for GitHub language detection and build convenience
.PHONY: all build clean

all: build

build:
	@if not exist build mkdir build
	cd build && cmake .. && cmake --build .

clean:
	if exist build rd /s /q build
