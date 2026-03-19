function createAnimal() {
    return { name: "", age: 0 };
}

function createDog() {
    return { ...createAnimal(), breed: "" };
}

function createPuppy() {
    return { ...createDog(), weight: 0 };
}

function describe(d) {
    console.log(d.name);
    console.log(d.age);
    console.log(d.breed);
}

function main() {
    let a = createAnimal();
    a.name = "Cat";
    a.age = 5;
    console.log(a.name);
    console.log(a.age);
    let d = createDog();
    d.name = "Rex";
    d.age = 3;
    d.breed = "Labrador";
    console.log(d.name);
    console.log(d.age);
    console.log(d.breed);
    describe(d);
    let p = createPuppy();
    p.name = "Tiny";
    p.age = 1;
    p.breed = "Beagle";
    p.weight = 8;
    console.log(p.name);
    console.log(p.age);
    console.log(p.breed);
    console.log(p.weight);
}


main();
