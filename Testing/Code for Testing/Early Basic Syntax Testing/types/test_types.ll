%Point = type { i32, i32 }
%Person = type { i8*, i32, %Point }

@.str.0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.1 = private unnamed_addr constant [4 x i8] c"Bob\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"%s\0A\00"

declare i32 @printf(i8*, ...)

define void @print_point(%Point %p.arg) {
entry:
  %p.addr = alloca %Point
  store %Point %p.arg, %Point* %p.addr
  %t0 = getelementptr %Point, %Point* %p.addr, i32 0, i32 0
  %t1 = load i32, i32* %t0
  %t2 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t3 = call i32 (i8*, ...) @printf(i8* %t2, i32 %t1)
  %t4 = getelementptr %Point, %Point* %p.addr, i32 0, i32 1
  %t5 = load i32, i32* %t4
  %t6 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t7 = call i32 (i8*, ...) @printf(i8* %t6, i32 %t5)
  ret void
}

define %Point @make_point(i32 %x.arg, i32 %y.arg) {
entry:
  %x.addr = alloca i32
  store i32 %x.arg, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y.arg, i32* %y.addr
  %result.addr = alloca %Point
  %t0 = load i32, i32* %x.addr
  %t1 = getelementptr %Point, %Point* %result.addr, i32 0, i32 0
  store i32 %t0, i32* %t1
  %t2 = load i32, i32* %y.addr
  %t3 = getelementptr %Point, %Point* %result.addr, i32 0, i32 1
  store i32 %t2, i32* %t3
  %t4 = load %Point, %Point* %result.addr
  ret %Point %t4
}

define void @main() {
entry:
  %p.addr = alloca %Point
  %t0 = getelementptr %Point, %Point* %p.addr, i32 0, i32 0
  store i32 10, i32* %t0
  %t1 = getelementptr %Point, %Point* %p.addr, i32 0, i32 1
  store i32 20, i32* %t1
  %t2 = getelementptr %Point, %Point* %p.addr, i32 0, i32 0
  %t3 = load i32, i32* %t2
  %t4 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t5 = call i32 (i8*, ...) @printf(i8* %t4, i32 %t3)
  %t6 = getelementptr %Point, %Point* %p.addr, i32 0, i32 1
  %t7 = load i32, i32* %t6
  %t8 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t9 = call i32 (i8*, ...) @printf(i8* %t8, i32 %t7)
  %bob.addr = alloca %Person
  %t10 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t11 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 0
  store i8* %t10, i8** %t11
  %t12 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 1
  store i32 30, i32* %t12
  %t13 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 2
  %t14 = getelementptr %Point, %Point* %t13, i32 0, i32 0
  store i32 100, i32* %t14
  %t15 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 2
  %t16 = getelementptr %Point, %Point* %t15, i32 0, i32 1
  store i32 200, i32* %t16
  %t17 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 0
  %t18 = load i8*, i8** %t17
  %t19 = getelementptr [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %t20 = call i32 (i8*, ...) @printf(i8* %t19, i8* %t18)
  %t21 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 1
  %t22 = load i32, i32* %t21
  %t23 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t24 = call i32 (i8*, ...) @printf(i8* %t23, i32 %t22)
  %t25 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 2
  %t26 = getelementptr %Point, %Point* %t25, i32 0, i32 0
  %t27 = load i32, i32* %t26
  %t28 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t29 = call i32 (i8*, ...) @printf(i8* %t28, i32 %t27)
  %t30 = getelementptr %Person, %Person* %bob.addr, i32 0, i32 2
  %t31 = getelementptr %Point, %Point* %t30, i32 0, i32 1
  %t32 = load i32, i32* %t31
  %t33 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t34 = call i32 (i8*, ...) @printf(i8* %t33, i32 %t32)
  %t35 = load %Point, %Point* %p.addr
  call void @print_point(%Point %t35)
  %q.addr = alloca %Point
  %t36 = call %Point @make_point(i32 5, i32 15)
  store %Point %t36, %Point* %q.addr
  %t37 = getelementptr %Point, %Point* %q.addr, i32 0, i32 0
  %t38 = load i32, i32* %t37
  %t39 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t40 = call i32 (i8*, ...) @printf(i8* %t39, i32 %t38)
  %t41 = getelementptr %Point, %Point* %q.addr, i32 0, i32 1
  %t42 = load i32, i32* %t41
  %t43 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t44 = call i32 (i8*, ...) @printf(i8* %t43, i32 %t42)
  ret void
}

