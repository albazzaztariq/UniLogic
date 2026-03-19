@.str.0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"

declare i32 @printf(i8*, ...)

define i32 @add(i32 %a.arg, i32 %b.arg) {
entry:
  %a.addr = alloca i32
  store i32 %a.arg, i32* %a.addr
  %b.addr = alloca i32
  store i32 %b.arg, i32* %b.addr
  %t0 = load i32, i32* %a.addr
  %t1 = load i32, i32* %b.addr
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}

define i32 @main() {
entry:
  %x.addr = alloca i32
  %t0 = call i32 @add(i32 3, i32 7)
  store i32 %t0, i32* %x.addr
  %t1 = load i32, i32* %x.addr
  %t2 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t3 = call i32 (i8*, ...) @printf(i8* %t2, i32 %t1)
  ret i32 0
}

