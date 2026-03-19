#!/usr/bin/env node
// Tools/run_wasm.js — Run a .wasm binary using Node.js WASI
// Usage: node run_wasm.js <file.wasm>

const { WASI } = require('wasi');
const { readFileSync } = require('fs');
const { argv } = require('process');

const wasmPath = argv[2];
if (!wasmPath) {
    console.error('Usage: node run_wasm.js <file.wasm>');
    process.exit(1);
}

try {
    const wasi = new WASI({ version: 'preview1' });
    const wasmBytes = readFileSync(wasmPath);
    const wasmModule = new WebAssembly.Module(wasmBytes);
    const instance = new WebAssembly.Instance(wasmModule, wasi.getImportObject());
    wasi.start(instance);
} catch (err) {
    console.error(`WASM runtime error: ${err.message}`);
    process.exit(1);
}
