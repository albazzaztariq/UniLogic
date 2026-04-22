@.str.0 = private unnamed_addr constant [4 x i8] c"one\00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%s\0A\00"
@.str.2 = private unnamed_addr constant [4 x i8] c"two\00"
@.str.3 = private unnamed_addr constant [6 x i8] c"three\00"
@.str.4 = private unnamed_addr constant [6 x i8] c"other\00"
@.str.5 = private unnamed_addr constant [12 x i8] c"default hit\00"
@.str.6 = private unnamed_addr constant [5 x i8] c"four\00"
@.str.7 = private unnamed_addr constant [5 x i8] c"five\00"
@.str.8 = private unnamed_addr constant [4 x i8] c"six\00"
@.str.9 = private unnamed_addr constant [13 x i8] c"negative one\00"
@.str.10 = private unnamed_addr constant [5 x i8] c"zero\00"
@.str.11 = private unnamed_addr constant [13 x i8] c"positive one\00"

declare i32 @printf(i8*, ...)

define void @test_int_match() {
entry:
  %x.addr = alloca i32
  store i32 2, i32* %x.addr
  %t0 = load i32, i32* %x.addr
  switch i32 %t0, label %match.default.1 [ i32 1, label %match.case.2 i32 2, label %match.case.3 i32 3, label %match.case.4 ]
match.case.2:
  %t1 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t2 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t3 = call i32 (i8*, ...) @printf(i8* %t2, i8* %t1)
  br label %match.end.0
match.case.3:
  %t4 = getelementptr [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %t5 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t6 = call i32 (i8*, ...) @printf(i8* %t5, i8* %t4)
  br label %match.end.0
match.case.4:
  %t7 = getelementptr [6 x i8], [6 x i8]* @.str.3, i32 0, i32 0
  %t8 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t9 = call i32 (i8*, ...) @printf(i8* %t8, i8* %t7)
  br label %match.end.0
match.default.1:
  %t10 = getelementptr [6 x i8], [6 x i8]* @.str.4, i32 0, i32 0
  %t11 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t12 = call i32 (i8*, ...) @printf(i8* %t11, i8* %t10)
  br label %match.end.0
match.end.0:
  ret void
}

define void @test_default() {
entry:
  %y.addr = alloca i32
  store i32 99, i32* %y.addr
  %t0 = load i32, i32* %y.addr
  switch i32 %t0, label %match.default.1 [ i32 1, label %match.case.2 i32 2, label %match.case.3 ]
match.case.2:
  %t1 = getelementptr [4 x i8], [4 x i8]* @.str.0, i32 0, i32 0
  %t2 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t3 = call i32 (i8*, ...) @printf(i8* %t2, i8* %t1)
  br label %match.end.0
match.case.3:
  %t4 = getelementptr [4 x i8], [4 x i8]* @.str.2, i32 0, i32 0
  %t5 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t6 = call i32 (i8*, ...) @printf(i8* %t5, i8* %t4)
  br label %match.end.0
match.default.1:
  %t7 = getelementptr [12 x i8], [12 x i8]* @.str.5, i32 0, i32 0
  %t8 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t9 = call i32 (i8*, ...) @printf(i8* %t8, i8* %t7)
  br label %match.end.0
match.end.0:
  ret void
}

define void @test_match_expr() {
entry:
  %val.addr = alloca i32
  %t0 = add i32 3, 2
  store i32 %t0, i32* %val.addr
  %t1 = load i32, i32* %val.addr
  switch i32 %t1, label %match.end.0 [ i32 4, label %match.case.1 i32 5, label %match.case.2 i32 6, label %match.case.3 ]
match.case.1:
  %t2 = getelementptr [5 x i8], [5 x i8]* @.str.6, i32 0, i32 0
  %t3 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t4 = call i32 (i8*, ...) @printf(i8* %t3, i8* %t2)
  br label %match.end.0
match.case.2:
  %t5 = getelementptr [5 x i8], [5 x i8]* @.str.7, i32 0, i32 0
  %t6 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t7 = call i32 (i8*, ...) @printf(i8* %t6, i8* %t5)
  br label %match.end.0
match.case.3:
  %t8 = getelementptr [4 x i8], [4 x i8]* @.str.8, i32 0, i32 0
  %t9 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t10 = call i32 (i8*, ...) @printf(i8* %t9, i8* %t8)
  br label %match.end.0
match.end.0:
  ret void
}

define void @test_match_negative() {
entry:
  %n.addr = alloca i32
  %t0 = sub i32 0, 1
  store i32 %t0, i32* %n.addr
  %t1 = load i32, i32* %n.addr
  %t2 = sub i32 0, 1
  %t3 = icmp eq i32 %t1, %t2
  br i1 %t3, label %match.case.1, label %match.case.2
match.case.1:
  %t4 = getelementptr [13 x i8], [13 x i8]* @.str.9, i32 0, i32 0
  %t5 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t6 = call i32 (i8*, ...) @printf(i8* %t5, i8* %t4)
  br label %match.end.0
  %t7 = icmp eq i32 %t1, 0
  br i1 %t7, label %match.case.2, label %match.case.3
match.case.2:
  %t8 = getelementptr [5 x i8], [5 x i8]* @.str.10, i32 0, i32 0
  %t9 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t10 = call i32 (i8*, ...) @printf(i8* %t9, i8* %t8)
  br label %match.end.0
  %t11 = icmp eq i32 %t1, 1
  br i1 %t11, label %match.case.3, label %match.end.0
match.case.3:
  %t12 = getelementptr [13 x i8], [13 x i8]* @.str.11, i32 0, i32 0
  %t13 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t14 = call i32 (i8*, ...) @printf(i8* %t13, i8* %t12)
  br label %match.end.0
match.end.0:
  ret void
}

define i32 @main() {
entry:
  call void @test_int_match()
  call void @test_default()
  call void @test_match_expr()
  call void @test_match_negative()
  ret i32 0
}

