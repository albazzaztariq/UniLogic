function str_char_at(s, i) { return i >= 0 && i < s.length ? s[i] : ''; }
function str_index_of(s, sub) { return s.indexOf(sub); }
function str_substr(s, start, len) { return s.substr(start, len); }
function strlen(s) { return s.length; }


function parse_row(row, out) {
    let field_count = 0;
    let pos = 0;
    let row_len = strlen(row);
    let current = "";
    let in_quotes = false;
    while ((pos < row_len)) {
        let ch = str_char_at(row, pos);
        if (in_quotes) {
            if ((ch === "\"")) {
                in_quotes = false;
            } else {
                current = (current + ch);
            }
        } else {
            if ((ch === "\"")) {
                in_quotes = true;
            } else {
                if ((ch === ",")) {
                    out[field_count] = current;
                    field_count = (field_count + 1);
                    current = "";
                } else {
                    if ((ch !== "\n")) {
                        current = (current + ch);
                    }
                }
            }
        }
        pos = (pos + 1);
    }
    out[field_count] = current;
    field_count = (field_count + 1);
    return field_count;
}

function split_rows(csv, out) {
    let row_count = 0;
    let pos = 0;
    let csv_len = strlen(csv);
    let current = "";
    while ((pos < csv_len)) {
        let ch = str_char_at(csv, pos);
        if ((ch === "\n")) {
            if ((strlen(current) > 0)) {
                out[row_count] = current;
                row_count = (row_count + 1);
            }
            current = "";
        } else {
            current = (current + ch);
        }
        pos = (pos + 1);
    }
    if ((strlen(current) > 0)) {
        out[row_count] = current;
        row_count = (row_count + 1);
    }
    return row_count;
}

function get_cell(rows, row, col) {
    let fields = ["", "", "", "", "", "", "", ""];
    parse_row(rows[row], fields);
    return fields[col];
}

function get_column_index(header_row, col_name) {
    let headers = ["", "", "", "", "", "", "", ""];
    let count = parse_row(header_row, headers);
    for (const i of __ul_range(count)) {
        if ((headers[i] === col_name)) {
            return i;
        }
    }
    return (-1);
}

function main() {
    let csv = "Name,Age,City,Score\nAlice,30,New York,95\nBob,25,\"Los Angeles, CA\",87\nCharlie,35,Chicago,92\nDiana,28,Houston,88\nEve,32,\"Portland, OR\",91\n";
    let rows = ["", "", "", "", "", "", "", "", "", ""];
    let num_rows = split_rows(csv, rows);
    console.log("Total rows (including header):");
    console.log(num_rows);
    console.log("");
    console.log("=== CSV Data ===");
    let headers = ["", "", "", "", "", "", "", ""];
    parse_row(rows[0], headers);
    console.log(headers[0]);
    console.log(headers[1]);
    console.log(headers[2]);
    console.log(headers[3]);
    console.log("");
    for (const i of __ul_range(1, num_rows)) {
        let fields = ["", "", "", "", "", "", "", ""];
        parse_row(rows[i], fields);
        console.log(fields[0]);
        console.log(fields[1]);
        console.log(fields[2]);
        console.log(fields[3]);
        console.log("---");
    }
    console.log("");
    console.log("=== Cell Lookups ===");
    let cell1 = get_cell(rows, 1, 0);
    console.log("Row 1, Col 0 (Name):");
    console.log(cell1);
    let cell2 = get_cell(rows, 2, 2);
    console.log("Row 2, Col 2 (City):");
    console.log(cell2);
    let cell3 = get_cell(rows, 4, 3);
    console.log("Row 4, Col 3 (Score):");
    console.log(cell3);
    console.log("");
    console.log("=== Column by Name ===");
    let city_col = get_column_index(rows[0], "City");
    console.log("City column index:");
    console.log(city_col);
    console.log("All cities:");
    for (const i of __ul_range(1, num_rows)) {
        let city = get_cell(rows, i, city_col);
        console.log(city);
    }
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
