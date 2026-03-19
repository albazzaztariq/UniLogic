%Animal = type { i8*, i32 }
%Dog = type { i8*, i32, i8* }
%Puppy = type { i8*, i32, i8*, i32 }

@.str.0 = private unnamed_addr constant [4 x i8] c"%s\0A\00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"Cat\00"
@.str.3 = private unnamed_addr constant [4 x i8] c"Rex\00"
@.str.4 = private unnamed_addr constant [9 x i8] c"Labrador\00"
@.str.5 = private unnamed_addr constant [5 x i8] c"Tiny\00"
@.str.6 = private unnamed_addr constant [7 x i8] c"Beagle\00"

declare i32 @printf(i8*, ...)

define void @describe(%Dog %d.arg) {
entry:
  %d.addr = alloca %Dog
  store %Dog %d.arg, %Dog* %d.addr
  %t0 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 0
  %t1 = load i8*, i8** %t0
  %t2 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t3 = call i32 (i8*, ...) @printf(i8* %t2, i8* %t1)
  %t4 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 1
  %t5 = load i32, i32* %t4
  %t6 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t7 = call i32 (i8*, ...) @printf(i8* %t6, i32 %t5)
  %t8 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 2
  %t9 = load i8*, i8** %t8
  %t10 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t11 = call i32 (i8*, ...) @printf(i8* %t10, i8* %t9)
  ret void
}

define void @main() {
entry:
  %a.addr = alloca %Animal
  %t0 = getelementptr [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %t1 = getelementptr %Animal, %Animal* %a.addr, i32 0, i32 0
  store i8* %t0, i8** %t1
  %t2 = getelementptr %Animal, %Animal* %a.addr, i32 0, i32 1
  store i32 5, i32* %t2
  %t3 = getelementptr %Animal, %Animal* %a.addr, i32 0, i32 0
  %t4 = load i8*, i8** %t3
  %t5 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t6 = call i32 (i8*, ...) @printf(i8* %t5, i8* %t4)
  %t7 = getelementptr %Animal, %Animal* %a.addr, i32 0, i32 1
  %t8 = load i32, i32* %t7
  %t9 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t10 = call i32 (i8*, ...) @printf(i8* %t9, i32 %t8)
  %d.addr = alloca %Dog
  %t11 = getelementptr [4 x i8], [4 x i8]* @.str.3, i32 0, i32 0
  %t12 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 0
  store i8* %t11, i8** %t12
  %t13 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 1
  store i32 3, i32* %t13
  %t14 = getelementptr [9 x i8], [9 x i8]* @.str.4, i32 0, i32 0
  %t15 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 2
  store i8* %t14, i8** %t15
  %t16 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 0
  %t17 = load i8*, i8** %t16
  %t18 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t19 = call i32 (i8*, ...) @printf(i8* %t18, i8* %t17)
  %t20 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 1
  %t21 = load i32, i32* %t20
  %t22 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t23 = call i32 (i8*, ...) @printf(i8* %t22, i32 %t21)
  %t24 = getelementptr %Dog, %Dog* %d.addr, i32 0, i32 2
  %t25 = load i8*, i8** %t24
  %t26 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t27 = call i32 (i8*, ...) @printf(i8* %t26, i8* %t25)
  %t28 = load %Dog, %Dog* %d.addr
  call void @describe(%Dog %t28)
  %p.addr = alloca %Puppy
  %t29 = getelementptr [5 x i8], [5 x i8]* @.str.5, i32 0, i32 0
  %t30 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 0
  store i8* %t29, i8** %t30
  %t31 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 1
  store i32 1, i32* %t31
  %t32 = getelementptr [7 x i8], [7 x i8]* @.str.6, i32 0, i32 0
  %t33 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 2
  store i8* %t32, i8** %t33
  %t34 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 3
  store i32 8, i32* %t34
  %t35 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 0
  %t36 = load i8*, i8** %t35
  %t37 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t38 = call i32 (i8*, ...) @printf(i8* %t37, i8* %t36)
  %t39 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 1
  %t40 = load i32, i32* %t39
  %t41 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t42 = call i32 (i8*, ...) @printf(i8* %t41, i32 %t40)
  %t43 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 2
  %t44 = load i8*, i8** %t43
  %t45 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t46 = call i32 (i8*, ...) @printf(i8* %t45, i8* %t44)
  %t47 = getelementptr %Puppy, %Puppy* %p.addr, i32 0, i32 3
  %t48 = load i32, i32* %t47
  %t49 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t50 = call i32 (i8*, ...) @printf(i8* %t49, i32 %t48)
  ret void
}

