#!/usr/bin/env bash
set -euo pipefail

BASE_URL="http://localhost:8080/api/v1"
TOTAL=${1:-100000}     # default 100k ops if not specified
CONCURRENCY=${2:-1}    # default single-threaded

echo "======================================"
echo " KV Throughput Benchmark"
echo " Total ops   : $TOTAL"
echo " Concurrency : $CONCURRENCY"
echo "======================================"

########################################
# PUT benchmark
########################################
echo
echo "[PUT] Starting insertion benchmark..."

start_ns=$(date +%s%N)

seq 1 "$TOTAL" | xargs -n1 -P"$CONCURRENCY" -I{} \
  curl -s -X PUT \
    "$BASE_URL/PUT?key=bench{}&value={}" \
    > /dev/null

end_ns=$(date +%s%N)

put_ns=$((end_ns - start_ns))
put_sec=$(echo "scale=6; $put_ns / 1000000000" | bc)
put_rps=$(echo "scale=2; $TOTAL / $put_sec" | bc)

echo "[PUT] Total time  : ${put_sec}s"
echo "[PUT] Throughput : ${put_rps} ops/sec"

########################################
# GET benchmark
########################################
echo
echo "[GET] Starting read benchmark..."

start_ns=$(date +%s%N)

seq 1 "$TOTAL" | xargs -n1 -P"$CONCURRENCY" -I{} \
  curl -s \
    "$BASE_URL/GET?key=bench{}" \
    > /dev/null

end_ns=$(date +%s%N)

get_ns=$((end_ns - start_ns))
get_sec=$(echo "scale=6; $get_ns / 1000000000" | bc)
get_rps=$(echo "scale=2; $TOTAL / $get_sec" | bc)

echo "[GET] Total time  : ${get_sec}s"
echo "[GET] Throughput : ${get_rps} ops/sec"

echo
echo "======================================"
echo " DONE"
echo "======================================"

# ./kv_throughput.sh 100000 8