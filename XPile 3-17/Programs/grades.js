class Student {
    constructor(name = "", grade1 = 0, grade2 = 0, grade3 = 0, grade4 = 0, grade5 = 0) {
        this.name = name;
        this.grade1 = grade1;
        this.grade2 = grade2;
        this.grade3 = grade3;
        this.grade4 = grade4;
        this.grade5 = grade5;
    }
    average() {
        let sum = ((((this.grade1 + this.grade2) + this.grade3) + this.grade4) + this.grade5);
        return Math.trunc(sum / 5);
    }
    highest() {
        let best = this.grade1;
        if ((this.grade2 > best)) {
            best = this.grade2;
        }
        if ((this.grade3 > best)) {
            best = this.grade3;
        }
        if ((this.grade4 > best)) {
            best = this.grade4;
        }
        if ((this.grade5 > best)) {
            best = this.grade5;
        }
        return best;
    }
    lowest() {
        let worst = this.grade1;
        if ((this.grade2 < worst)) {
            worst = this.grade2;
        }
        if ((this.grade3 < worst)) {
            worst = this.grade3;
        }
        if ((this.grade4 < worst)) {
            worst = this.grade4;
        }
        if ((this.grade5 < worst)) {
            worst = this.grade5;
        }
        return worst;
    }
    letter_grade() {
        let avg = ((((this.grade1 + this.grade2) + this.grade3) + this.grade4) + this.grade5);
        avg = Math.trunc(avg / 5);
        if ((avg >= 90)) {
            return "A";
        }
        if ((avg >= 80)) {
            return "B";
        }
        if ((avg >= 70)) {
            return "C";
        }
        if ((avg >= 60)) {
            return "D";
        }
        return "F";
    }
    is_passing() {
        let avg = ((((this.grade1 + this.grade2) + this.grade3) + this.grade4) + this.grade5);
        avg = Math.trunc(avg / 5);
        return (avg >= 60);
    }
}


function print_student(s) {
    console.log(s.name);
    console.log("  Average:");
    console.log(s.average());
    console.log("  Grade:");
    console.log(s.letter_grade());
    console.log("  Highest:");
    console.log(s.highest());
    console.log("  Lowest:");
    console.log(s.lowest());
    let status = "PASS";
    if ((s.is_passing() === false)) {
        status = "FAIL";
    }
    console.log("  Status:");
    console.log(status);
    console.log("");
}

function main() {
    let s1 = new Student();
    s1.name = "Alice";
    s1.grade1 = 95;
    s1.grade2 = 88;
    s1.grade3 = 92;
    s1.grade4 = 97;
    s1.grade5 = 90;
    let s2 = new Student();
    s2.name = "Bob";
    s2.grade1 = 72;
    s2.grade2 = 65;
    s2.grade3 = 78;
    s2.grade4 = 70;
    s2.grade5 = 68;
    let s3 = new Student();
    s3.name = "Charlie";
    s3.grade1 = 85;
    s3.grade2 = 90;
    s3.grade3 = 88;
    s3.grade4 = 82;
    s3.grade5 = 91;
    let s4 = new Student();
    s4.name = "Diana";
    s4.grade1 = 55;
    s4.grade2 = 60;
    s4.grade3 = 45;
    s4.grade4 = 58;
    s4.grade5 = 52;
    let s5 = new Student();
    s5.name = "Eve";
    s5.grade1 = 98;
    s5.grade2 = 95;
    s5.grade3 = 100;
    s5.grade4 = 97;
    s5.grade5 = 99;
    let s6 = new Student();
    s6.name = "Frank";
    s6.grade1 = 40;
    s6.grade2 = 35;
    s6.grade3 = 42;
    s6.grade4 = 38;
    s6.grade5 = 45;
    let s7 = new Student();
    s7.name = "Grace";
    s7.grade1 = 78;
    s7.grade2 = 82;
    s7.grade3 = 75;
    s7.grade4 = 80;
    s7.grade5 = 77;
    let s8 = new Student();
    s8.name = "Hank";
    s8.grade1 = 88;
    s8.grade2 = 91;
    s8.grade3 = 85;
    s8.grade4 = 90;
    s8.grade5 = 86;
    console.log("=== Student Grade Report ===");
    console.log("");
    print_student(s1);
    print_student(s2);
    print_student(s3);
    print_student(s4);
    print_student(s5);
    print_student(s6);
    print_student(s7);
    print_student(s8);
    console.log("=== Class Statistics ===");
    let total_avg = (((((((s1.average() + s2.average()) + s3.average()) + s4.average()) + s5.average()) + s6.average()) + s7.average()) + s8.average());
    let cavg = Math.trunc(total_avg / 8);
    console.log("Class average:");
    console.log(cavg);
    let best = s1.average();
    let best_name = "Alice";
    if ((s2.average() > best)) {
        best = s2.average();
        best_name = "Bob";
    }
    if ((s3.average() > best)) {
        best = s3.average();
        best_name = "Charlie";
    }
    if ((s4.average() > best)) {
        best = s4.average();
        best_name = "Diana";
    }
    if ((s5.average() > best)) {
        best = s5.average();
        best_name = "Eve";
    }
    if ((s6.average() > best)) {
        best = s6.average();
        best_name = "Frank";
    }
    if ((s7.average() > best)) {
        best = s7.average();
        best_name = "Grace";
    }
    if ((s8.average() > best)) {
        best = s8.average();
        best_name = "Hank";
    }
    console.log("Top student:");
    console.log(best_name);
    let failing = 0;
    if ((s1.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s2.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s3.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s4.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s5.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s6.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s7.is_passing() === false)) {
        failing = (failing + 1);
    }
    if ((s8.is_passing() === false)) {
        failing = (failing + 1);
    }
    console.log("Failing students:");
    console.log(failing);
    return 0;
}

function __ul_sort(arr, n) { const sub = arr.slice(0, n).sort((a, b) => a < b ? -1 : a > b ? 1 : 0); for (let i = 0; i < n; i++) arr[i] = sub[i]; }

main();
