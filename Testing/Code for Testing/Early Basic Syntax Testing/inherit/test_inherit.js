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
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
