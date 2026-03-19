function greet(lang) {
    switch (lang) {
        case "en":
            console.log("hello");
            break;
        case "es":
            console.log("hola");
            break;
        case "fr":
            console.log("bonjour");
            break;
        case "de":
            console.log("hallo");
            break;
        default:
            console.log("unknown");
            break;
    }
}

function main() {
    greet("en");
    greet("es");
    greet("fr");
    greet("de");
    greet("jp");
    let code = "es";
    switch (code) {
        case "en":
            console.log("english");
            break;
        case "es":
            console.log("spanish");
            break;
        default:
            console.log("other");
            break;
    }
    return 0;
}


main();
