# WebAssembly C++ Demo

This project demonstrates compiling a simple C++ program to WebAssembly and running it in the browser with JavaScript and HTML.


## How to Run

python3 -m http.server 8000

## Recompiling

If you want to modify a.cpp and recompile:

bash
emcc a.cpp -o app.js -s WASM=1