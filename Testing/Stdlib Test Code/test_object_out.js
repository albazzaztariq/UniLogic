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


main();
