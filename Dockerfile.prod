# ================
# BUILD STAGE
# ================
FROM gcc:13 AS builder
WORKDIR /build

COPY makefile .
COPY src ./src
COPY include ./include

RUN make

# ===============
# RUNTIME STAGE
# ===============
FROM debian:bookworm-slim

WORKDIR /app
# we will copy the compiled binaries here
COPY --from=builder /build/myapp .

# Running as non-root
RUN useradd -m appuser
USER appuser

CMD ["./myapp"]