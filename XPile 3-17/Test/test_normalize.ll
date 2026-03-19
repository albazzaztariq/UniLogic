@.str.0 = private unnamed_addr constant [4 x i8] c"%d\0A\00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%f\0A\00"

declare i32 @printf(i8*, ...)
declare double @sqrt(double)

define i32 @main() {
entry:
  %counter.addr = alloca i32
  store i32 10, i32* %counter.addr
  %t0 = load i32, i32* %counter.addr
  %t1 = add i32 %t0, 5
  store i32 %t1, i32* %counter.addr
  %t2 = load i32, i32* %counter.addr
  %t3 = mul i32 %t2, 2
  store i32 %t3, i32* %counter.addr
  %i.addr = alloca i32
  store i32 0, i32* %i.addr
  %t4 = load i32, i32* %i.addr
  %t5 = add i32 %t4, 1
  store i32 %t5, i32* %i.addr
  %t6 = load i32, i32* %i.addr
  %t7 = add i32 %t6, 1
  store i32 %t7, i32* %i.addr
  %t8 = load i32, i32* %i.addr
  %t9 = sub i32 %t8, 1
  store i32 %t9, i32* %i.addr
  %t10 = load i32, i32* %counter.addr
  %t11 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t12 = call i32 (i8*, ...) @printf(i8* %t11, i32 %t10)
  %t13 = load i32, i32* %i.addr
  %t14 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t15 = call i32 (i8*, ...) @printf(i8* %t14, i32 %t13)
  %val.addr = alloca double
  store double 144.0, double* %val.addr
  %deep.addr = alloca double
  %t16 = load double, double* %val.addr
  %t17 = call double @sqrt(double %t16)
  %t18 = call double @sqrt(double %t17)
  store double %t18, double* %deep.addr
  %t19 = load double, double* %deep.addr
  %t20 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t21 = call i32 (i8*, ...) @printf(i8* %t20, double %t19)
  %x.addr = alloca i32
  store i32 42, i32* %x.addr
  %y.addr = alloca double
  %t22 = load i32, i32* %x.addr
  %t23 = sitofp i32 %t22 to float
  %t24 = fpext float %t23 to double
  store double %t24, double* %y.addr
  %t25 = load double, double* %y.addr
  %t26 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t27 = call i32 (i8*, ...) @printf(i8* %t26, double %t25)
  %a.addr = alloca i32
  store i32 1, i32* %a.addr
  %b.addr = alloca i32
  store i32 2, i32* %b.addr
  %c.addr = alloca i32
  store i32 3, i32* %c.addr
  %t28 = load i32, i32* %a.addr
  %t29 = icmp sgt i32 %t28, 0
  %t30 = load i32, i32* %b.addr
  %t31 = icmp sgt i32 %t30, 0
  %t32 = and i1 %t29, %t31
  %t33 = load i32, i32* %c.addr
  %t34 = icmp sgt i32 %t33, 0
  %t35 = and i1 %t32, %t34
  br i1 %t35, label %if.then.0, label %if.end.1
if.then.0:
  %t36 = load i32, i32* %a.addr
  %t37 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t38 = call i32 (i8*, ...) @printf(i8* %t37, i32 %t36)
  br label %if.end.1
if.end.1:
  ret i32 0
}

