// @dr concurrency = threaded

function worker(id) {
    console.log(id);
}

function main() {
    spawn(worker(1));
    spawn(worker(2));
    spawn(worker(3));
    wait();
    return 0;
}


main();
