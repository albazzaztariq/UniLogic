from dataclasses import dataclass, field
def strlen(s): return len(s)

def __ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class __ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def repeat_char(ch, n):
    result = ''
    for i in range(n):
        result = (result + ch)
    return result

def pad_str(s, width):
    slen = strlen(s)
    result = s
    pad = (width - slen)
    for i in range(pad):
        result = (result + ' ')
    return result

def build_line(widths, cols, begin_s, fill, sep, end_s):
    line = begin_s
    for c in range(cols):
        line = (line + repeat_char(fill, widths[c]))
        if (c < (cols - 1)):
            line = (line + sep)
    line = (line + end_s)
    return line

def build_row(cells, widths, cols, begin_s, sep, end_s, padding):
    line = begin_s
    for c in range(cols):
        for p in range(padding):
            line = (line + ' ')
        line = (line + pad_str(cells[c], widths[c]))
        for p in range(padding):
            line = (line + ' ')
        if (c < (cols - 1)):
            line = (line + sep)
    line = (line + end_s)
    return line

def tabulate(data, rows, cols, headers, fmt):
    widths = [0, 0, 0, 0, 0, 0, 0, 0]
    for c in range(cols):
        widths[c] = strlen(headers[c])
    for r in range(rows):
        for c in range(cols):
            cell_len = strlen(data[((r * cols) + c)])
            if (cell_len > widths[c]):
                widths[c] = cell_len
    if (fmt == 0):
        padded = [0, 0, 0, 0, 0, 0, 0, 0]
        for c in range(cols):
            padded[c] = (widths[c] + 2)
        print(build_line(padded, cols, '+', '-', '+', '+'))
        print(build_row(headers, widths, cols, '|', '|', '|', 1))
        print(build_line(padded, cols, '+', '=', '+', '+'))
        for r in range(rows):
            row_cells = ['', '', '', '', '', '', '', '']
            for c in range(cols):
                row_cells[c] = data[((r * cols) + c)]
            print(build_row(row_cells, widths, cols, '|', '|', '|', 1))
            if (r < (rows - 1)):
                print(build_line(padded, cols, '+', '-', '+', '+'))
        print(build_line(padded, cols, '+', '-', '+', '+'))
    else:
        print(build_row(headers, widths, cols, '', '  ', '', 0))
        print(build_line(widths, cols, '', '-', '  ', ''))
        for r in range(rows):
            row_cells = ['', '', '', '', '', '', '', '']
            for c in range(cols):
                row_cells[c] = data[((r * cols) + c)]
            print(build_row(row_cells, widths, cols, '', '  ', '', 0))

def main():
    print('=== Test 1: Grid format ===')
    h1 = ['Name', 'Age', 'City', '', '', '', '', '']
    t1 = ['Alice', '30', 'New York', 'Bob', '25', 'Chicago', 'Charlie', '35', 'Houston', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '']
    tabulate(t1, 3, 3, h1, 0)
    print('')
    print('=== Test 2: Simple format ===')
    tabulate(t1, 3, 3, h1, 1)
    print('')
    print('=== Test 3: Wide columns ===')
    h3 = ['Item', 'Description', 'Price', 'Qty', '', '', '', '']
    t3 = ['Widget', 'A small widget', '9.99', '100', 'Gizmo', 'Large industrial gizmo', '249.50', '5', 'Bolt', 'M8 hex bolt', '0.50', '10000', '', '', '', '', '', '', '', '', '', '', '', '']
    tabulate(t3, 3, 4, h3, 0)
    return 0


if __name__ == "__main__":
    main()
