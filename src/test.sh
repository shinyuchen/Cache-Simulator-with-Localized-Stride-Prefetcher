#!/bin/bash

mips_args=(--icache=128:2:128:2 --dcache=64:4:128:2 --l2cache=128:8:128:50 --memspeed=100)
alpha_args=(--icache=512:2:64:2 --dcache=256:4:64:2 --l2cache=16384:8:64:50 --memspeed=100)

# run <expected.txt> <trace.bz2> <cache args>
function run() {
    local expected="${1}"
    local trace="${2}"
    shift 2
    local args=("$@")

    echo "running:" "${expected}"
    diff --color=always \
        <(bunzip2 -kc "${trace}" | ./cache "${args[@]}" | grep -v '^Student') \
        <(grep -v '^Student' "${expected}")
}

function main() {
    local trace_name
    for trace_name in bzip2 gcc h264 namd; do
        run "../correctOutput/mips/${trace_name}.txt" \
            "../traces/${trace_name}.bz2" \
            "${mips_args[@]}"
        run "../correctOutput/alpha/${trace_name}.txt" \
            "../traces/${trace_name}.bz2" \
            "${alpha_args[@]}"
    done
}

main