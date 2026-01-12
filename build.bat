@echo off
emcc cs_snake_web.c -O2 ^
  -sWASM=1 ^
  -sMODULARIZE=1 ^
  -sEXPORT_ES6=1 ^
  -sEXPORTED_RUNTIME_METHODS="['cwrap','ccall']" ^
  -o snake.js
echo Build done. Outputs: snake.js, snake.wasm
