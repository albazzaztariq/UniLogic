@.str.0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define void @_gen_body_0(i32 %val.arg) {
entry:
  %val.addr = alloca i32
  store i32 %val.arg, i32* %val.addr
  %t0 = load i32, i32* %val.addr
  %t1 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t2 = call i32 (i8*, ...) @printf(i8* %t1, i32 %t0)
  ret void
}

define void @_gen_body_1(i32 %d.arg) {
entry:
  %d.addr = alloca i32
  store i32 %d.arg, i32* %d.addr
  %t0 = load i32, i32* %d.addr
  %t1 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t2 = call i32 (i8*, ...) @printf(i8* %t1, i32 %t0)
  ret void
}

define void @countdown(i32 %n.arg, void (i32)* %_yield_cb) {
entry:
  %n.addr = alloca i32
  store i32 %n.arg, i32* %n.addr
  br label %while.cond.0
while.cond.0:
  %t0 = load i32, i32* %n.addr
  %t1 = icmp sgt i32 %t0, 0
  br i1 %t1, label %while.body.1, label %while.end.2
while.body.1:
  %t2 = load i32, i32* %n.addr
  call void %_yield_cb(i32 %t2)
  %t3 = load i32, i32* %n.addr
  %t4 = sub i32 %t3, 1
  store i32 %t4, i32* %n.addr
  br label %while.cond.0
while.end.2:
  ret void
}

define void @doubles(i32 %n.arg, void (i32)* %_yield_cb) {
entry:
  %n.addr = alloca i32
  store i32 %n.arg, i32* %n.addr
  %i.addr = alloca i32
  store i32 1, i32* %i.addr
  br label %while.cond.0
while.cond.0:
  %t0 = load i32, i32* %i.addr
  %t1 = load i32, i32* %n.addr
  %t2 = icmp sle i32 %t0, %t1
  br i1 %t2, label %while.body.1, label %while.end.2
while.body.1:
  %t3 = load i32, i32* %i.addr
  %t4 = mul i32 %t3, 2
  call void %_yield_cb(i32 %t4)
  %t5 = load i32, i32* %i.addr
  %t6 = add i32 %t5, 1
  store i32 %t6, i32* %i.addr
  br label %while.cond.0
while.end.2:
  ret void
}

define void @main() {
entry:
  call void @countdown(i32 5, void (i32)* @_gen_body_0)
  call void @doubles(i32 4, void (i32)* @_gen_body_1)
  ret void
}

