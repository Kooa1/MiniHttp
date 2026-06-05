#!/bin/bash

URL="${1:-http://localhost:8080}"
DURATION=10
REPORT="bench-report-$(date +%Y%m%d-%H%M%S).md"

# ── tool detection ──
TOOL=""
if command -v wrk &>/dev/null; then TOOL="wrk"
elif command -v ab &>/dev/null; then TOOL="ab"
else
    echo "ERROR: neither wrk nor ab found."
    echo "  sudo apt install wrk"
    exit 1
fi
echo "Tool: $TOOL"

# ── pre-check ──
if ! curl -sf -o /dev/null "$URL/"; then
    echo "ERROR: server not reachable at $URL"
    echo "Start the server first, then re-run."
    exit 1
fi
echo "Server: $URL  (alive)"
echo ""

# ── scenarios ──
declare -a SCENARIOS=(
    "GET /|/"
    "GET /about|/about"
    "GET /slow|/slow"
    "GET static/index.html|/static/"
    "GET static/style.css|/static/css/style.css"
    "GET /api/routes|/api/routes"
    "GET /user/1234|/user/1234"
)
CONCURRENCIES=(1 10 50 100)

# ── helpers ──
# threads = min(concurrency, 2) so -c1 won't break
run_wrk() {
    local url="$1" c="$2"
    local t=$(( c < 2 ? c : 2 ))
    wrk -t"$t" -c"$c" -d"${DURATION}s" --latency "$url" 2>&1
}

run_ab() {
    ab -c "$2" -t "$DURATION" -k "$1" 2>&1
}

# parse the single "Latency" stat line (not "Latency Distribution")
_lat_stat() {
    echo "$1" | grep -E 'Latency[[:space:]]+[0-9]'
}

parse_wrk() {
    local raw="$1"
    local lat_line
    lat_line=$(_lat_stat "$raw")
    rps=$(echo "$raw" | grep 'Requests/sec:' | awk '{print $2}')
    lat_avg=$(echo "$lat_line" | awk '{print $2}')
    lat_max=$(echo "$lat_line" | awk '{print $4}')
    p50=$(echo "$raw" | grep '50%' | awk '{print $2}')
    p99=$(echo "$raw" | grep '99%' | awk '{print $2}')
    # Non-2xx: wrk prints "Non-2xx or 3xx responses: N"
    non2xx=$(echo "$raw" | grep 'Non-2xx' | awk '{print $NF}')
    echo "$rps|$lat_avg|$lat_max|$p50|$p99|${non2xx:-0}"
}

parse_ab() {
    local raw="$1"
    rps=$(echo "$raw" | grep 'Requests per second:' | awk '{print $4}')
    lat_avg=$(echo "$raw" | grep '(mean, across all concurrent requests)' | awk '{print $4}')
    echo "$rps|${lat_avg:-N/A}|N/A|N/A|N/A|0"
}

# ── init report ──
{
    echo "# MiniHttp Load Test Report"
    echo ""
    echo "Date: $(date '+%Y-%m-%d %H:%M')"
    echo "Tool: $TOOL"
    echo "Server: $URL"
    echo "Duration: ${DURATION}s per test"
    echo ""
    echo "## Results"
    echo ""
    echo "| Scenario | Concurrency | RPS | Avg Latency | Max Latency | p50 | p99 | Non-2xx |"
    echo "|----------|-------------|-----|-------------|-------------|-----|-----|---------|"
} > "$REPORT"

results_rows=()

# ── run tests ──
for entry in "${SCENARIOS[@]}"; do
    name="${entry%%|*}"
    endpoint="${entry##*|}"
    full_url="${URL}${endpoint}"
    echo ""
    echo "━━━ $name ━━━"

    for c in "${CONCURRENCIES[@]}"; do
        echo -n "  [$c] "

        if [ "$TOOL" = "wrk" ]; then
            raw=$(run_wrk "$full_url" "$c")
        else
            raw=$(run_ab "$full_url" "$c")
        fi

        # save raw output
        {
            echo "======== $name concurrency=$c ========"
            echo "$raw"
            echo ""
        } >> "$REPORT.raw"

        if [ "$TOOL" = "wrk" ]; then
            parsed=$(parse_wrk "$raw")
        else
            parsed=$(parse_ab "$raw")
        fi

        IFS='|' read -r rps lat_avg lat_max p50 p99 non2xx <<< "$parsed"
        echo "RPS=$rps  avg=$lat_avg  max=$lat_max  p50=$p50  p99=$p99  non2xx=$non2xx"
        results_rows+=("| $name | $c | $rps | $lat_avg | $lat_max | $p50 | $p99 | $non2xx |")
    done
done

# ── write results ──
for row in "${results_rows[@]}"; do
    echo "$row" >> "$REPORT"
done

# ── append raw ──
{
    echo ""
    echo "## Raw Output"
    echo ""
    echo '```'
    cat "$REPORT.raw"
    echo '```'
} >> "$REPORT"

rm -f "$REPORT.raw"

echo ""
echo "════════════════════════════"
echo "Report: $REPORT"
echo "════════════════════════════"
