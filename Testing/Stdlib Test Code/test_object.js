class Animal {
    constructor(name = "", age = 0) {
        this.name = name;
        this.age = age;
    }
    speak() {
        return "...";
    }
}

class Dog extends Animal {
    constructor(name = "", age = 0, breed = "") {
        super(name, age);
        this.breed = breed;
    }
    speak() {
        return "Woof!";
    }
    info() {
        return this.breed;
    }
}


function main() {
    let d = new Dog();
    d.name = "Rex";
    d.age = 3;
    d.breed = "Labrador";
    console.log(d.name);
    console.log(d.age);
    console.log(d.speak());
    console.log(d.info());
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
