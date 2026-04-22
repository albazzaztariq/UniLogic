function createPoint() {
    return { x: 0, y: 0 };
}

class Animal {
    constructor(name = "", age = 0) {
        this.name = name;
        this.age = age;
    }
    speak() {
        return (this.name + " makes a sound");
    }
}

class Dog extends Animal {
    constructor(name = "", age = 0, breed = "") {
        super(name, age);
        this.breed = breed;
    }
    speak() {
        return (this.name + " barks");
    }
    fetch() {
        return (this.name + " fetches the ball");
    }
}

class Cat extends Animal {
    constructor(name = "", age = 0) {
        super(name, age);
    }
    speak() {
        return (this.name + " meows");
    }
}


function main() {
    let p = {x: 10, y: 20};
    console.log(p.x);
    console.log(p.y);
    let a = new Animal();
    a.name = "Generic";
    a.age = 5;
    console.log(a.speak());
    let d = new Dog();
    d.name = "Rex";
    d.age = 3;
    d.breed = "Labrador";
    console.log(d.speak());
    console.log(d.fetch());
    let c = new Cat();
    c.name = "Whiskers";
    c.age = 7;
    console.log(c.speak());
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
