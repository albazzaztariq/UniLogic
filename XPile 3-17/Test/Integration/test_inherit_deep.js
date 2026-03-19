function createAnimal() {
    return { name: "", age: 0 };
}

function createDog() {
    return { ...createAnimal(), breed: "" };
}

function createPuppy() {
    return { ...createDog(), weight: 0 };
}

function show_animal(a) {
    console.log(a.name);
    console.log(a.age);
}

function show_dog(d) {
    console.log(d.name);
    console.log(d.age);
    console.log(d.breed);
}

function show_puppy(p) {
    console.log(p.name);
    console.log(p.age);
    console.log(p.breed);
    console.log(p.weight);
}

function main() {
    let a = createAnimal();
    a.name = "Tiger";
    a.age = 10;
    show_animal(a);
    let d = createDog();
    d.name = "Rex";
    d.age = 5;
    d.breed = "Husky";
    show_dog(d);
    let p = createPuppy();
    p.name = "Tiny";
    p.age = 1;
    p.breed = "Poodle";
    p.weight = 3;
    show_puppy(p);
    p.name = "Spot";
    p.age = 2;
    p.breed = "Beagle";
    p.weight = 5;
    show_puppy(p);
    return 0;
}


main();
