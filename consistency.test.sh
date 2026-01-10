#!/usr/bin/env bash
set -euo pipefail

BASE_URL="http://localhost:8080/api/v1"
N=4000

echo "[TEST] Sequential PUT ($N keys)"

for ((i=1;i<=N;i++)); do
  curl -s -X PUT \
    "$BASE_URL/PUT?key=key$i&value=$i" > /dev/null

  if (( i % 100000 == 0 )); then
    echo "  inserted $i"
  fi
done

echo "[TEST] Random GET verification (10k samples)"

for ((i=1;i<=N;i++)); do
  k=$(( RANDOM % N + 1 ))
  val=$(curl -s "$BASE_URL/GET?key=key$k" | jq -r .value)

  if [[ "$val" != "$k" ]]; then
    echo "❌ CONSISTENCY FAILURE: key$k expected=$k got=$val"
    exit 1
  fi
done

echo "✅ Random GET consistency passed"

echo "[TEST] Random DELETE + GET check"

for ((i=1;i<=N;i++)); do
  k=$(( RANDOM % N + 1 ))
  curl -s -X DELETE "$BASE_URL/DELETE?key=key$k" > /dev/null

  code=$(curl -s -o /dev/null -w "%{http_code}" \
    "$BASE_URL/GET?key=key$k")

  if [[ "$code" != "404" ]]; then
    echo "❌ DELETE FAILED for key$k"
    exit 1
  fi
done

echo "✅ DELETE consistency passed"
echo "🎉 ALL TESTS PASSED"