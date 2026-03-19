@.str.0 = private unnamed_addr constant [4 x i8] c"%f\0A\00"
@.str.1 = private unnamed_addr constant [16 x i8] c"Hello from FFI!\00"

declare i32 @printf(i8*, ...)
declare double @sqrt(double)
declare i32 @puts(i8*)

define i32 @main() {
entry:
  %val.addr = alloca double
  store double 144.0, double* %val.addr
  %result.addr = alloca double
  %t0 = load double, double* %val.addr
  %t1 = call double @sqrt(double %t0)
  store double %t1, double* %result.addr
  %t2 = load double, double* %result.addr
  %t3 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t4 = call i32 (i8*, ...) @printf(i8* %t3, double %t2)
  %t5 = getelementptr [16 x i8], [16 x i8]* @.str.1, i32 0, i32 0
  %t6 = call i32 @puts(i8* %t5)
  ret i32 0
}

