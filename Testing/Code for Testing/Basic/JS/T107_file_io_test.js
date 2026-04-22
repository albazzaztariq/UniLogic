
function main() {
    let content = "line one";
    console.log(content);
    let doc = (((("header" + "\n") + "body") + "\n") + "footer");
    console.log(doc);
    let path = "output.txt";
    console.log(("path: " + path));
    let data = "10,20,30";
    console.log(data);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
