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

# ── pre-check: is server alive? ──
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
run_wrk() {
    wrk -t2 -c"$2" -d"${DURATION}s" --latency "$1" 2>&1
}

run_ab() {
    ab -c "$2" -t "$DURATION" -k "$1" 2>&1
}

parse_wrk() {
    local raw="$1"
    local rps lat_avg lat_max
    rps=$(echo "$raw" | grep 'Requests/sec:' | awk '{print $2}')
    lat_avg=$(echo "$raw" | grep 'Latency' | awk '{print $2}')
    lat_max=$(echo "$raw" | grep 'Latency' | awk '{print $4}')
    echo "$rps|$lat_avg|$lat_max"
}

parse_ab() {
    local raw="$1"
    local rps lat_avg
    rps=$(echo "$raw" | grep 'Requests per second:' | awk '{print $4}')
    lat_avg=$(echo "$raw" | grep '(mean, across all concurrent requests)' | awk '{print $4}')
    echo "$rps|${lat_avg:-N/A}|N/A"
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
    echo "| Scenario | Concurrency | RPS | Avg Latency | Max Latency |"
    echo "|----------|-------------|-----|-------------|-------------|"
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

        # save raw for report
        echo "======== $name concurrency=$c ========" >> "$REPORT.raw"
        echo "$raw" >> "$REPORT.raw"
        echo "" >> "$REPORT.raw"

        if [ "$TOOL" = "wrk" ]; then
            parsed=$(parse_wrk "$raw")
        else
            parsed=$(parse_ab "$raw")
        fi

        rps=$(echo "$parsed" | cut -d'|' -f1)
        lat_avg=$(echo "$parsed" | cut -d'|' -f2)
        lat_max=$(echo "$parsed" | cut -d'|' -f3)

        echo "RPS=$rps  avg=$lat_avg  max=$lat_max"
        results_rows+=("| $name | $c | $rps | $lat_avg | $lat_max |")
    done
done

# ── write results ──
for row in "${results_rows[@]}"; do
    echo "$row" >> "$REPORT"
done

# ── append raw ──
echo "" >> "$REPORT"
echo "## Raw Output" >> "$REPORT"
echo "" >> "$REPORT"
echo '```' >> "$REPORT"
cat "$REPORT.raw" >> "$REPORT"
echo '```' >> "$REPORT"

rm -f "$REPORT.raw"

echo ""
echo "════════════════════════════"
echo "Report: $REPORT"
echo "════════════════════════════"
