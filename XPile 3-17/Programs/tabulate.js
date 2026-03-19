function strlen(s) { return s.length; }


function repeat_char(ch, n) {
    let result = "";
    for (const i of __ul_range(n)) {
        result = (result + ch);
    }
    return result;
}

function pad_str(s, width) {
    let slen = strlen(s);
    let result = s;
    let pad = (width - slen);
    for (const i of __ul_range(pad)) {
        result = (result + " ");
    }
    return result;
}

function build_line(widths, cols, begin_s, fill, sep, end_s) {
    let line = begin_s;
    for (const c of __ul_range(cols)) {
        line = (line + repeat_char(fill, widths[c]));
        if ((c < (cols - 1))) {
            line = (line + sep);
        }
    }
    line = (line + end_s);
    return line;
}

function build_row(cells, widths, cols, begin_s, sep, end_s, padding) {
    let line = begin_s;
    for (const c of __ul_range(cols)) {
        for (const p of __ul_range(padding)) {
            line = (line + " ");
        }
        line = (line + pad_str(cells[c], widths[c]));
        for (const p of __ul_range(padding)) {
            line = (line + " ");
        }
        if ((c < (cols - 1))) {
            line = (line + sep);
        }
    }
    line = (line + end_s);
    return line;
}

function tabulate(data, rows, cols, headers, fmt) {
    let widths = [0, 0, 0, 0, 0, 0, 0, 0];
    for (const c of __ul_range(cols)) {
        widths[c] = strlen(headers[c]);
    }
    for (const r of __ul_range(rows)) {
        for (const c of __ul_range(cols)) {
            let cell_len = strlen(data[((r * cols) + c)]);
            if ((cell_len > widths[c])) {
                widths[c] = cell_len;
            }
        }
    }
    if ((fmt === 0)) {
        let padded = [0, 0, 0, 0, 0, 0, 0, 0];
        for (const c of __ul_range(cols)) {
            padded[c] = (widths[c] + 2);
        }
        console.log(build_line(padded, cols, "+", "-", "+", "+"));
        console.log(build_row(headers, widths, cols, "|", "|", "|", 1));
        console.log(build_line(padded, cols, "+", "=", "+", "+"));
        for (const r of __ul_range(rows)) {
            let row_cells = ["", "", "", "", "", "", "", ""];
            for (const c of __ul_range(cols)) {
                row_cells[c] = data[((r * cols) + c)];
            }
            console.log(build_row(row_cells, widths, cols, "|", "|", "|", 1));
            if ((r < (rows - 1))) {
                console.log(build_line(padded, cols, "+", "-", "+", "+"));
            }
        }
        console.log(build_line(padded, cols, "+", "-", "+", "+"));
    } else {
        console.log(build_row(headers, widths, cols, "", "  ", "", 0));
        console.log(build_line(widths, cols, "", "-", "  ", ""));
        for (const r of __ul_range(rows)) {
            let row_cells = ["", "", "", "", "", "", "", ""];
            for (const c of __ul_range(cols)) {
                row_cells[c] = data[((r * cols) + c)];
            }
            console.log(build_row(row_cells, widths, cols, "", "  ", "", 0));
        }
    }
}

function main() {
    console.log("=== Test 1: Grid format ===");
    let h1 = ["Name", "Age", "City", "", "", "", "", ""];
    let t1 = ["Alice", "30", "New York", "Bob", "25", "Chicago", "Charlie", "35", "Houston", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""];
    tabulate(t1, 3, 3, h1, 0);
    console.log("");
    console.log("=== Test 2: Simple format ===");
    tabulate(t1, 3, 3, h1, 1);
    console.log("");
    console.log("=== Test 3: Wide columns ===");
    let h3 = ["Item", "Description", "Price", "Qty", "", "", "", ""];
    let t3 = ["Widget", "A small widget", "9.99", "100", "Gizmo", "Large industrial gizmo", "249.50", "5", "Bolt", "M8 hex bolt", "0.50", "10000", "", "", "", "", "", "", "", "", "", "", "", ""];
    tabulate(t3, 3, 4, h3, 0);
    return 0;
}


function __ul_range(...args) {
    let start, end, step;
    if (args.length === 1) { start = 0; end = args[0]; step = 1; }
    else if (args.length === 2) { start = args[0]; end = args[1]; step = 1; }
    else { start = args[0]; end = args[1]; step = args[2]; }
    const result = [];
    for (let i = start; step > 0 ? i < end : i > end; i += step) result.push(i);
    return result;
}
function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
