CC = gcc
FLAGS = -O3
FLAGS += -lpthread

docker:
	docker build -t kvcache .
	docker run --rm kvcache
	docker build -f Dockerfile.test -t test-test .
	docker build -f Dockerfile.prod -t test-prod .
