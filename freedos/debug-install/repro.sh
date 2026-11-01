#!/bin/sh
set -eu
go build -o /tmp/ai386-debug-freedos-install ./build/freedos/debug-install
/tmp/ai386-debug-freedos-install <<'EOF'
{"Steps":20000000,"Until":"Welcome to FreeDOS 1.4","Require":"Welcome to FreeDOS 1.4"}
{"Steps":2000000,"Until":"Install to harddisk","Require":"Install to harddisk"}
{"Steps":2000000}
{"Keys":[57424],"Steps":200000}
{"Keys":[57424],"Steps":200000}
{"Keys":[28],"Steps":400000000,"Until":".Error!","Reject":".Error!"}
EOF
