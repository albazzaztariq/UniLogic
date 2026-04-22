@.str.0 = private unnamed_addr constant [15 x i8] c"adler32(\22\22) = \00"
@.str.1 = private unnamed_addr constant [4 x i8] c"%s\0A\00"
@.str.2 = private unnamed_addr constant [18 x i8] c"adler32(\22abc\22) = \00"
@.str.3 = private unnamed_addr constant [28 x i8] c"adler32(\22Hello, World!\22) = \00"
@.str.4 = private unnamed_addr constant [4 x i8] c"lo=\00"
@.str.5 = private unnamed_addr constant [5 x i8] c" hi=\00"
@.str.6 = private unnamed_addr constant [13 x i8] c" recombined=\00"
@.str.7 = private unnamed_addr constant [8 x i8] c" match=\00"
@.str.8 = private unnamed_addr constant [20 x i8] c"adler32(\22Hello\22) = \00"
@.str.9 = private unnamed_addr constant [23 x i8] c"adler32(\22, World!\22) = \00"
@.str.10 = private unnamed_addr constant [12 x i8] c"combined = \00"
@.str.11 = private unnamed_addr constant [12 x i8] c"direct   = \00"
@.str.12 = private unnamed_addr constant [9 x i8] c"match = \00"
@.str.13 = private unnamed_addr constant [13 x i8] c"bit_swap: x=\00"
@.str.14 = private unnamed_addr constant [4 x i8] c" y=\00"
@.str.15 = private unnamed_addr constant [4 x i8] c" z=\00"
@.str.16 = private unnamed_addr constant [4 x i8] c" w=\00"

declare i32 @printf(i8*, ...)

define i64 @adler32_hello_world() {
entry:
  %adler.addr = alloca i64
  %t0 = sext i32 1 to i64
  store i64 %t0, i64* %adler.addr
  %sum2.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %sum2.addr
  %b.addr = alloca i32
  store i32 0, i32* %b.addr
  store i32 72, i32* %b.addr
  %t2 = load i64, i64* %adler.addr
  %t3 = load i32, i32* %b.addr
  %t4 = sext i32 %t3 to i64
  %t5 = add i64 %t2, %t4
  store i64 %t5, i64* %adler.addr
  %t6 = load i64, i64* %sum2.addr
  %t7 = load i64, i64* %adler.addr
  %t8 = add i64 %t6, %t7
  store i64 %t8, i64* %sum2.addr
  store i32 101, i32* %b.addr
  %t9 = load i64, i64* %adler.addr
  %t10 = load i32, i32* %b.addr
  %t11 = sext i32 %t10 to i64
  %t12 = add i64 %t9, %t11
  store i64 %t12, i64* %adler.addr
  %t13 = load i64, i64* %sum2.addr
  %t14 = load i64, i64* %adler.addr
  %t15 = add i64 %t13, %t14
  store i64 %t15, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t16 = load i64, i64* %adler.addr
  %t17 = load i32, i32* %b.addr
  %t18 = sext i32 %t17 to i64
  %t19 = add i64 %t16, %t18
  store i64 %t19, i64* %adler.addr
  %t20 = load i64, i64* %sum2.addr
  %t21 = load i64, i64* %adler.addr
  %t22 = add i64 %t20, %t21
  store i64 %t22, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t23 = load i64, i64* %adler.addr
  %t24 = load i32, i32* %b.addr
  %t25 = sext i32 %t24 to i64
  %t26 = add i64 %t23, %t25
  store i64 %t26, i64* %adler.addr
  %t27 = load i64, i64* %sum2.addr
  %t28 = load i64, i64* %adler.addr
  %t29 = add i64 %t27, %t28
  store i64 %t29, i64* %sum2.addr
  store i32 111, i32* %b.addr
  %t30 = load i64, i64* %adler.addr
  %t31 = load i32, i32* %b.addr
  %t32 = sext i32 %t31 to i64
  %t33 = add i64 %t30, %t32
  store i64 %t33, i64* %adler.addr
  %t34 = load i64, i64* %sum2.addr
  %t35 = load i64, i64* %adler.addr
  %t36 = add i64 %t34, %t35
  store i64 %t36, i64* %sum2.addr
  store i32 44, i32* %b.addr
  %t37 = load i64, i64* %adler.addr
  %t38 = load i32, i32* %b.addr
  %t39 = sext i32 %t38 to i64
  %t40 = add i64 %t37, %t39
  store i64 %t40, i64* %adler.addr
  %t41 = load i64, i64* %sum2.addr
  %t42 = load i64, i64* %adler.addr
  %t43 = add i64 %t41, %t42
  store i64 %t43, i64* %sum2.addr
  store i32 32, i32* %b.addr
  %t44 = load i64, i64* %adler.addr
  %t45 = load i32, i32* %b.addr
  %t46 = sext i32 %t45 to i64
  %t47 = add i64 %t44, %t46
  store i64 %t47, i64* %adler.addr
  %t48 = load i64, i64* %sum2.addr
  %t49 = load i64, i64* %adler.addr
  %t50 = add i64 %t48, %t49
  store i64 %t50, i64* %sum2.addr
  store i32 87, i32* %b.addr
  %t51 = load i64, i64* %adler.addr
  %t52 = load i32, i32* %b.addr
  %t53 = sext i32 %t52 to i64
  %t54 = add i64 %t51, %t53
  store i64 %t54, i64* %adler.addr
  %t55 = load i64, i64* %sum2.addr
  %t56 = load i64, i64* %adler.addr
  %t57 = add i64 %t55, %t56
  store i64 %t57, i64* %sum2.addr
  store i32 111, i32* %b.addr
  %t58 = load i64, i64* %adler.addr
  %t59 = load i32, i32* %b.addr
  %t60 = sext i32 %t59 to i64
  %t61 = add i64 %t58, %t60
  store i64 %t61, i64* %adler.addr
  %t62 = load i64, i64* %sum2.addr
  %t63 = load i64, i64* %adler.addr
  %t64 = add i64 %t62, %t63
  store i64 %t64, i64* %sum2.addr
  store i32 114, i32* %b.addr
  %t65 = load i64, i64* %adler.addr
  %t66 = load i32, i32* %b.addr
  %t67 = sext i32 %t66 to i64
  %t68 = add i64 %t65, %t67
  store i64 %t68, i64* %adler.addr
  %t69 = load i64, i64* %sum2.addr
  %t70 = load i64, i64* %adler.addr
  %t71 = add i64 %t69, %t70
  store i64 %t71, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t72 = load i64, i64* %adler.addr
  %t73 = load i32, i32* %b.addr
  %t74 = sext i32 %t73 to i64
  %t75 = add i64 %t72, %t74
  store i64 %t75, i64* %adler.addr
  %t76 = load i64, i64* %sum2.addr
  %t77 = load i64, i64* %adler.addr
  %t78 = add i64 %t76, %t77
  store i64 %t78, i64* %sum2.addr
  store i32 100, i32* %b.addr
  %t79 = load i64, i64* %adler.addr
  %t80 = load i32, i32* %b.addr
  %t81 = sext i32 %t80 to i64
  %t82 = add i64 %t79, %t81
  store i64 %t82, i64* %adler.addr
  %t83 = load i64, i64* %sum2.addr
  %t84 = load i64, i64* %adler.addr
  %t85 = add i64 %t83, %t84
  store i64 %t85, i64* %sum2.addr
  store i32 33, i32* %b.addr
  %t86 = load i64, i64* %adler.addr
  %t87 = load i32, i32* %b.addr
  %t88 = sext i32 %t87 to i64
  %t89 = add i64 %t86, %t88
  store i64 %t89, i64* %adler.addr
  %t90 = load i64, i64* %sum2.addr
  %t91 = load i64, i64* %adler.addr
  %t92 = add i64 %t90, %t91
  store i64 %t92, i64* %sum2.addr
  %t93 = load i64, i64* %adler.addr
  %t94 = sext i32 65521 to i64
  %t95 = srem i64 %t93, %t94
  store i64 %t95, i64* %adler.addr
  %t96 = load i64, i64* %sum2.addr
  %t97 = sext i32 65521 to i64
  %t98 = srem i64 %t96, %t97
  store i64 %t98, i64* %sum2.addr
  %t99 = load i64, i64* %adler.addr
  %t100 = load i64, i64* %sum2.addr
  %t101 = sext i32 16 to i64
  ; unknown binop 'left'
  %t103 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t105 = sext i32 0 to i64
  ret i64 %t105
}

define i64 @adler32_empty() {
entry:
  %t0 = sext i32 1 to i64
  ret i64 %t0
}

define i64 @adler32_abc() {
entry:
  %adler.addr = alloca i64
  %t0 = sext i32 1 to i64
  store i64 %t0, i64* %adler.addr
  %sum2.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %sum2.addr
  %b.addr = alloca i32
  store i32 0, i32* %b.addr
  store i32 97, i32* %b.addr
  %t2 = load i64, i64* %adler.addr
  %t3 = load i32, i32* %b.addr
  %t4 = sext i32 %t3 to i64
  %t5 = add i64 %t2, %t4
  store i64 %t5, i64* %adler.addr
  %t6 = load i64, i64* %sum2.addr
  %t7 = load i64, i64* %adler.addr
  %t8 = add i64 %t6, %t7
  store i64 %t8, i64* %sum2.addr
  store i32 98, i32* %b.addr
  %t9 = load i64, i64* %adler.addr
  %t10 = load i32, i32* %b.addr
  %t11 = sext i32 %t10 to i64
  %t12 = add i64 %t9, %t11
  store i64 %t12, i64* %adler.addr
  %t13 = load i64, i64* %sum2.addr
  %t14 = load i64, i64* %adler.addr
  %t15 = add i64 %t13, %t14
  store i64 %t15, i64* %sum2.addr
  store i32 99, i32* %b.addr
  %t16 = load i64, i64* %adler.addr
  %t17 = load i32, i32* %b.addr
  %t18 = sext i32 %t17 to i64
  %t19 = add i64 %t16, %t18
  store i64 %t19, i64* %adler.addr
  %t20 = load i64, i64* %sum2.addr
  %t21 = load i64, i64* %adler.addr
  %t22 = add i64 %t20, %t21
  store i64 %t22, i64* %sum2.addr
  %t23 = load i64, i64* %adler.addr
  %t24 = sext i32 65521 to i64
  %t25 = srem i64 %t23, %t24
  store i64 %t25, i64* %adler.addr
  %t26 = load i64, i64* %sum2.addr
  %t27 = sext i32 65521 to i64
  %t28 = srem i64 %t26, %t27
  store i64 %t28, i64* %sum2.addr
  %t29 = load i64, i64* %adler.addr
  %t30 = load i64, i64* %sum2.addr
  %t31 = sext i32 16 to i64
  ; unknown binop 'left'
  %t33 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t35 = sext i32 0 to i64
  ret i64 %t35
}

define i64 @adler32_lo(i64 %adler.arg) {
entry:
  %adler.addr = alloca i64
  store i64 %adler.arg, i64* %adler.addr
  %t0 = load i64, i64* %adler.addr
  %t1 = sext i32 65535 to i64
  ; unknown binop 'both1'
  %t3 = sext i32 0 to i64
  ret i64 %t3
}

define i64 @adler32_hi(i64 %adler.arg) {
entry:
  %adler.addr = alloca i64
  store i64 %adler.arg, i64* %adler.addr
  %t0 = load i64, i64* %adler.addr
  %t1 = sext i32 16 to i64
  ; unknown binop 'right'
  ; unknown binop 'both1'
  %t4 = sext i32 0 to i64
  ret i64 %t4
}

define i64 @adler32_combine(i64 %adler1.arg, i64 %adler2.arg, i64 %len2.arg) {
entry:
  %adler1.addr = alloca i64
  store i64 %adler1.arg, i64* %adler1.addr
  %adler2.addr = alloca i64
  store i64 %adler2.arg, i64* %adler2.addr
  %len2.addr = alloca i64
  store i64 %len2.arg, i64* %len2.addr
  %sum1.addr = alloca i64
  %t0 = sext i32 0 to i64
  store i64 %t0, i64* %sum1.addr
  %sum2.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %sum2.addr
  %rem.addr = alloca i64
  %t2 = sext i32 0 to i64
  store i64 %t2, i64* %rem.addr
  %t3 = load i64, i64* %len2.addr
  %t4 = sext i32 65521 to i64
  %t5 = srem i64 %t3, %t4
  store i64 %t5, i64* %rem.addr
  %t6 = load i64, i64* %adler1.addr
  %t7 = sext i32 65535 to i64
  ; unknown binop 'both1'
  %t9 = sext i32 0 to i64
  store i64 %t9, i64* %sum1.addr
  %t10 = load i64, i64* %rem.addr
  %t11 = load i64, i64* %sum1.addr
  %t12 = mul i64 %t10, %t11
  store i64 %t12, i64* %sum2.addr
  %t13 = load i64, i64* %sum2.addr
  %t14 = sext i32 65521 to i64
  %t15 = srem i64 %t13, %t14
  store i64 %t15, i64* %sum2.addr
  %t16 = load i64, i64* %sum1.addr
  %t17 = load i64, i64* %adler2.addr
  %t18 = sext i32 65535 to i64
  ; unknown binop 'both1'
  %t20 = sext i32 0 to i64
  %t21 = add i64 %t16, %t20
  %t22 = sext i32 65521 to i64
  %t23 = add i64 %t21, %t22
  %t24 = sext i32 1 to i64
  %t25 = sub i64 %t23, %t24
  store i64 %t25, i64* %sum1.addr
  %t26 = load i64, i64* %sum2.addr
  %t27 = load i64, i64* %adler1.addr
  %t28 = sext i32 16 to i64
  ; unknown binop 'right'
  ; unknown binop 'both1'
  %t31 = sext i32 0 to i64
  %t32 = add i64 %t26, %t31
  %t33 = load i64, i64* %adler2.addr
  %t34 = sext i32 16 to i64
  ; unknown binop 'right'
  ; unknown binop 'both1'
  %t37 = sext i32 0 to i64
  %t38 = add i64 %t32, %t37
  %t39 = sext i32 65521 to i64
  %t40 = add i64 %t38, %t39
  %t41 = load i64, i64* %rem.addr
  %t42 = sub i64 %t40, %t41
  store i64 %t42, i64* %sum2.addr
  %t43 = load i64, i64* %sum1.addr
  %t44 = sext i32 65521 to i64
  %t45 = icmp sge i64 %t43, %t44
  br i1 %t45, label %if.then.0, label %if.end.1
if.then.0:
  %t46 = load i64, i64* %sum1.addr
  %t47 = sext i32 65521 to i64
  %t48 = sub i64 %t46, %t47
  store i64 %t48, i64* %sum1.addr
  br label %if.end.1
if.end.1:
  %t49 = load i64, i64* %sum1.addr
  %t50 = sext i32 65521 to i64
  %t51 = icmp sge i64 %t49, %t50
  br i1 %t51, label %if.then.2, label %if.end.3
if.then.2:
  %t52 = load i64, i64* %sum1.addr
  %t53 = sext i32 65521 to i64
  %t54 = sub i64 %t52, %t53
  store i64 %t54, i64* %sum1.addr
  br label %if.end.3
if.end.3:
  %t55 = load i64, i64* %sum2.addr
  %t56 = mul i32 65521, 2
  %t57 = sext i32 %t56 to i64
  %t58 = icmp sge i64 %t55, %t57
  br i1 %t58, label %if.then.4, label %if.end.5
if.then.4:
  %t59 = load i64, i64* %sum2.addr
  %t60 = mul i32 65521, 2
  %t61 = sext i32 %t60 to i64
  %t62 = sub i64 %t59, %t61
  store i64 %t62, i64* %sum2.addr
  br label %if.end.5
if.end.5:
  %t63 = load i64, i64* %sum2.addr
  %t64 = sext i32 65521 to i64
  %t65 = icmp sge i64 %t63, %t64
  br i1 %t65, label %if.then.6, label %if.end.7
if.then.6:
  %t66 = load i64, i64* %sum2.addr
  %t67 = sext i32 65521 to i64
  %t68 = sub i64 %t66, %t67
  store i64 %t68, i64* %sum2.addr
  br label %if.end.7
if.end.7:
  %t69 = load i64, i64* %sum1.addr
  %t70 = load i64, i64* %sum2.addr
  %t71 = sext i32 16 to i64
  ; unknown binop 'left'
  %t73 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t75 = sext i32 0 to i64
  ret i64 %t75
}

define i64 @adler32_hello() {
entry:
  %adler.addr = alloca i64
  %t0 = sext i32 1 to i64
  store i64 %t0, i64* %adler.addr
  %sum2.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %sum2.addr
  %b.addr = alloca i32
  store i32 0, i32* %b.addr
  store i32 72, i32* %b.addr
  %t2 = load i64, i64* %adler.addr
  %t3 = load i32, i32* %b.addr
  %t4 = sext i32 %t3 to i64
  %t5 = add i64 %t2, %t4
  store i64 %t5, i64* %adler.addr
  %t6 = load i64, i64* %sum2.addr
  %t7 = load i64, i64* %adler.addr
  %t8 = add i64 %t6, %t7
  store i64 %t8, i64* %sum2.addr
  store i32 101, i32* %b.addr
  %t9 = load i64, i64* %adler.addr
  %t10 = load i32, i32* %b.addr
  %t11 = sext i32 %t10 to i64
  %t12 = add i64 %t9, %t11
  store i64 %t12, i64* %adler.addr
  %t13 = load i64, i64* %sum2.addr
  %t14 = load i64, i64* %adler.addr
  %t15 = add i64 %t13, %t14
  store i64 %t15, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t16 = load i64, i64* %adler.addr
  %t17 = load i32, i32* %b.addr
  %t18 = sext i32 %t17 to i64
  %t19 = add i64 %t16, %t18
  store i64 %t19, i64* %adler.addr
  %t20 = load i64, i64* %sum2.addr
  %t21 = load i64, i64* %adler.addr
  %t22 = add i64 %t20, %t21
  store i64 %t22, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t23 = load i64, i64* %adler.addr
  %t24 = load i32, i32* %b.addr
  %t25 = sext i32 %t24 to i64
  %t26 = add i64 %t23, %t25
  store i64 %t26, i64* %adler.addr
  %t27 = load i64, i64* %sum2.addr
  %t28 = load i64, i64* %adler.addr
  %t29 = add i64 %t27, %t28
  store i64 %t29, i64* %sum2.addr
  store i32 111, i32* %b.addr
  %t30 = load i64, i64* %adler.addr
  %t31 = load i32, i32* %b.addr
  %t32 = sext i32 %t31 to i64
  %t33 = add i64 %t30, %t32
  store i64 %t33, i64* %adler.addr
  %t34 = load i64, i64* %sum2.addr
  %t35 = load i64, i64* %adler.addr
  %t36 = add i64 %t34, %t35
  store i64 %t36, i64* %sum2.addr
  %t37 = load i64, i64* %adler.addr
  %t38 = sext i32 65521 to i64
  %t39 = srem i64 %t37, %t38
  store i64 %t39, i64* %adler.addr
  %t40 = load i64, i64* %sum2.addr
  %t41 = sext i32 65521 to i64
  %t42 = srem i64 %t40, %t41
  store i64 %t42, i64* %sum2.addr
  %t43 = load i64, i64* %adler.addr
  %t44 = load i64, i64* %sum2.addr
  %t45 = sext i32 16 to i64
  ; unknown binop 'left'
  %t47 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t49 = sext i32 0 to i64
  ret i64 %t49
}

define i64 @adler32_world() {
entry:
  %adler.addr = alloca i64
  %t0 = sext i32 1 to i64
  store i64 %t0, i64* %adler.addr
  %sum2.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %sum2.addr
  %b.addr = alloca i32
  store i32 0, i32* %b.addr
  store i32 44, i32* %b.addr
  %t2 = load i64, i64* %adler.addr
  %t3 = load i32, i32* %b.addr
  %t4 = sext i32 %t3 to i64
  %t5 = add i64 %t2, %t4
  store i64 %t5, i64* %adler.addr
  %t6 = load i64, i64* %sum2.addr
  %t7 = load i64, i64* %adler.addr
  %t8 = add i64 %t6, %t7
  store i64 %t8, i64* %sum2.addr
  store i32 32, i32* %b.addr
  %t9 = load i64, i64* %adler.addr
  %t10 = load i32, i32* %b.addr
  %t11 = sext i32 %t10 to i64
  %t12 = add i64 %t9, %t11
  store i64 %t12, i64* %adler.addr
  %t13 = load i64, i64* %sum2.addr
  %t14 = load i64, i64* %adler.addr
  %t15 = add i64 %t13, %t14
  store i64 %t15, i64* %sum2.addr
  store i32 87, i32* %b.addr
  %t16 = load i64, i64* %adler.addr
  %t17 = load i32, i32* %b.addr
  %t18 = sext i32 %t17 to i64
  %t19 = add i64 %t16, %t18
  store i64 %t19, i64* %adler.addr
  %t20 = load i64, i64* %sum2.addr
  %t21 = load i64, i64* %adler.addr
  %t22 = add i64 %t20, %t21
  store i64 %t22, i64* %sum2.addr
  store i32 111, i32* %b.addr
  %t23 = load i64, i64* %adler.addr
  %t24 = load i32, i32* %b.addr
  %t25 = sext i32 %t24 to i64
  %t26 = add i64 %t23, %t25
  store i64 %t26, i64* %adler.addr
  %t27 = load i64, i64* %sum2.addr
  %t28 = load i64, i64* %adler.addr
  %t29 = add i64 %t27, %t28
  store i64 %t29, i64* %sum2.addr
  store i32 114, i32* %b.addr
  %t30 = load i64, i64* %adler.addr
  %t31 = load i32, i32* %b.addr
  %t32 = sext i32 %t31 to i64
  %t33 = add i64 %t30, %t32
  store i64 %t33, i64* %adler.addr
  %t34 = load i64, i64* %sum2.addr
  %t35 = load i64, i64* %adler.addr
  %t36 = add i64 %t34, %t35
  store i64 %t36, i64* %sum2.addr
  store i32 108, i32* %b.addr
  %t37 = load i64, i64* %adler.addr
  %t38 = load i32, i32* %b.addr
  %t39 = sext i32 %t38 to i64
  %t40 = add i64 %t37, %t39
  store i64 %t40, i64* %adler.addr
  %t41 = load i64, i64* %sum2.addr
  %t42 = load i64, i64* %adler.addr
  %t43 = add i64 %t41, %t42
  store i64 %t43, i64* %sum2.addr
  store i32 100, i32* %b.addr
  %t44 = load i64, i64* %adler.addr
  %t45 = load i32, i32* %b.addr
  %t46 = sext i32 %t45 to i64
  %t47 = add i64 %t44, %t46
  store i64 %t47, i64* %adler.addr
  %t48 = load i64, i64* %sum2.addr
  %t49 = load i64, i64* %adler.addr
  %t50 = add i64 %t48, %t49
  store i64 %t50, i64* %sum2.addr
  store i32 33, i32* %b.addr
  %t51 = load i64, i64* %adler.addr
  %t52 = load i32, i32* %b.addr
  %t53 = sext i32 %t52 to i64
  %t54 = add i64 %t51, %t53
  store i64 %t54, i64* %adler.addr
  %t55 = load i64, i64* %sum2.addr
  %t56 = load i64, i64* %adler.addr
  %t57 = add i64 %t55, %t56
  store i64 %t57, i64* %sum2.addr
  %t58 = load i64, i64* %adler.addr
  %t59 = sext i32 65521 to i64
  %t60 = srem i64 %t58, %t59
  store i64 %t60, i64* %adler.addr
  %t61 = load i64, i64* %sum2.addr
  %t62 = sext i32 65521 to i64
  %t63 = srem i64 %t61, %t62
  store i64 %t63, i64* %sum2.addr
  %t64 = load i64, i64* %adler.addr
  %t65 = load i64, i64* %sum2.addr
  %t66 = sext i32 16 to i64
  ; unknown binop 'left'
  %t68 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t70 = sext i32 0 to i64
  ret i64 %t70
}

define i32 @main() {
entry:
  %r.addr = alloca i64
  %t0 = sext i32 0 to i64
  store i64 %t0, i64* %r.addr
  %lo.addr = alloca i64
  %t1 = sext i32 0 to i64
  store i64 %t1, i64* %lo.addr
  %hi.addr = alloca i64
  %t2 = sext i32 0 to i64
  store i64 %t2, i64* %hi.addr
  %recombined.addr = alloca i64
  %t3 = sext i32 0 to i64
  store i64 %t3, i64* %recombined.addr
  %r1.addr = alloca i64
  %t4 = sext i32 0 to i64
  store i64 %t4, i64* %r1.addr
  %r2.addr = alloca i64
  %t5 = sext i32 0 to i64
  store i64 %t5, i64* %r2.addr
  %combined.addr = alloca i64
  %t6 = sext i32 0 to i64
  store i64 %t6, i64* %combined.addr
  %t7 = call i64 @adler32_empty()
  store i64 %t7, i64* %r.addr
  %t8 = getelementptr [15 x i8], [15 x i8]* @.str.0, i32 0, i32 0
  %t9 = load i64, i64* %r.addr
  %t10 = bitcast i64 %t9 to i8*
  %t11 = add i8* %t8, %t10
  %t12 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t13 = call i32 (i8*, ...) @printf(i8* %t12, i8* %t11)
  %t14 = call i64 @adler32_abc()
  store i64 %t14, i64* %r.addr
  %t15 = getelementptr [18 x i8], [18 x i8]* @.str.2, i32 0, i32 0
  %t16 = load i64, i64* %r.addr
  %t17 = bitcast i64 %t16 to i8*
  %t18 = add i8* %t15, %t17
  %t19 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t20 = call i32 (i8*, ...) @printf(i8* %t19, i8* %t18)
  %t21 = call i64 @adler32_hello_world()
  store i64 %t21, i64* %r.addr
  %t22 = getelementptr [28 x i8], [28 x i8]* @.str.3, i32 0, i32 0
  %t23 = load i64, i64* %r.addr
  %t24 = bitcast i64 %t23 to i8*
  %t25 = add i8* %t22, %t24
  %t26 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t27 = call i32 (i8*, ...) @printf(i8* %t26, i8* %t25)
  %t28 = load i64, i64* %r.addr
  %t29 = call i64 @adler32_lo(i64 %t28)
  store i64 %t29, i64* %lo.addr
  %t30 = load i64, i64* %r.addr
  %t31 = call i64 @adler32_hi(i64 %t30)
  store i64 %t31, i64* %hi.addr
  %t32 = load i64, i64* %lo.addr
  %t33 = load i64, i64* %hi.addr
  %t34 = sext i32 16 to i64
  ; unknown binop 'left'
  %t36 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t38 = sext i32 0 to i64
  store i64 %t38, i64* %recombined.addr
  %t39 = getelementptr [4 x i8], [4 x i8]* @.str.4, i32 0, i32 0
  %t40 = load i64, i64* %lo.addr
  %t41 = bitcast i64 %t40 to i8*
  %t42 = add i8* %t39, %t41
  %t43 = getelementptr [5 x i8], [5 x i8]* @.str.5, i32 0, i32 0
  %t44 = add i8* %t42, %t43
  %t45 = load i64, i64* %hi.addr
  %t46 = bitcast i64 %t45 to i8*
  %t47 = add i8* %t44, %t46
  %t48 = getelementptr [13 x i8], [13 x i8]* @.str.6, i32 0, i32 0
  %t49 = add i8* %t47, %t48
  %t50 = load i64, i64* %recombined.addr
  %t51 = bitcast i64 %t50 to i8*
  %t52 = add i8* %t49, %t51
  %t53 = getelementptr [8 x i8], [8 x i8]* @.str.7, i32 0, i32 0
  %t54 = add i8* %t52, %t53
  %t55 = load i64, i64* %recombined.addr
  %t56 = load i64, i64* %r.addr
  %t57 = icmp eq i64 %t55, %t56
  %t58 = bitcast i1 %t57 to i8*
  %t59 = add i8* %t54, %t58
  %t60 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t61 = call i32 (i8*, ...) @printf(i8* %t60, i8* %t59)
  %t62 = call i64 @adler32_hello()
  store i64 %t62, i64* %r1.addr
  %t63 = call i64 @adler32_world()
  store i64 %t63, i64* %r2.addr
  %t64 = load i64, i64* %r1.addr
  %t65 = load i64, i64* %r2.addr
  %t66 = sext i32 8 to i64
  %t67 = call i64 @adler32_combine(i64 %t64, i64 %t65, i64 %t66)
  store i64 %t67, i64* %combined.addr
  %t68 = getelementptr [20 x i8], [20 x i8]* @.str.8, i32 0, i32 0
  %t69 = load i64, i64* %r1.addr
  %t70 = bitcast i64 %t69 to i8*
  %t71 = add i8* %t68, %t70
  %t72 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t73 = call i32 (i8*, ...) @printf(i8* %t72, i8* %t71)
  %t74 = getelementptr [23 x i8], [23 x i8]* @.str.9, i32 0, i32 0
  %t75 = load i64, i64* %r2.addr
  %t76 = bitcast i64 %t75 to i8*
  %t77 = add i8* %t74, %t76
  %t78 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t79 = call i32 (i8*, ...) @printf(i8* %t78, i8* %t77)
  %t80 = getelementptr [12 x i8], [12 x i8]* @.str.10, i32 0, i32 0
  %t81 = load i64, i64* %combined.addr
  %t82 = bitcast i64 %t81 to i8*
  %t83 = add i8* %t80, %t82
  %t84 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t85 = call i32 (i8*, ...) @printf(i8* %t84, i8* %t83)
  %t86 = getelementptr [12 x i8], [12 x i8]* @.str.11, i32 0, i32 0
  %t87 = load i64, i64* %r.addr
  %t88 = bitcast i64 %t87 to i8*
  %t89 = add i8* %t86, %t88
  %t90 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t91 = call i32 (i8*, ...) @printf(i8* %t90, i8* %t89)
  %t92 = getelementptr [9 x i8], [9 x i8]* @.str.12, i32 0, i32 0
  %t93 = load i64, i64* %combined.addr
  %t94 = load i64, i64* %r.addr
  %t95 = icmp eq i64 %t93, %t94
  %t96 = bitcast i1 %t95 to i8*
  %t97 = add i8* %t92, %t96
  %t98 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t99 = call i32 (i8*, ...) @printf(i8* %t98, i8* %t97)
  %x.addr = alloca i64
  %t100 = sext i32 3735928559 to i64
  store i64 %t100, i64* %x.addr
  %y.addr = alloca i64
  %t101 = load i64, i64* %x.addr
  %t102 = sext i32 16 to i64
  ; unknown binop 'right'
  ; unknown binop 'both1'
  %t105 = sext i32 0 to i64
  store i64 %t105, i64* %y.addr
  %z.addr = alloca i64
  %t106 = load i64, i64* %x.addr
  %t107 = sext i32 65535 to i64
  ; unknown binop 'both1'
  %t109 = sext i32 0 to i64
  store i64 %t109, i64* %z.addr
  %w.addr = alloca i64
  %t110 = load i64, i64* %y.addr
  %t111 = load i64, i64* %z.addr
  %t112 = sext i32 16 to i64
  ; unknown binop 'left'
  %t114 = sext i32 0 to i64
  ; unknown binop 'either1'
  %t116 = sext i32 0 to i64
  store i64 %t116, i64* %w.addr
  %t117 = getelementptr [13 x i8], [13 x i8]* @.str.13, i32 0, i32 0
  %t118 = load i64, i64* %x.addr
  %t119 = bitcast i64 %t118 to i8*
  %t120 = add i8* %t117, %t119
  %t121 = getelementptr [4 x i8], [4 x i8]* @.str.14, i32 0, i32 0
  %t122 = add i8* %t120, %t121
  %t123 = load i64, i64* %y.addr
  %t124 = bitcast i64 %t123 to i8*
  %t125 = add i8* %t122, %t124
  %t126 = getelementptr [4 x i8], [4 x i8]* @.str.15, i32 0, i32 0
  %t127 = add i8* %t125, %t126
  %t128 = load i64, i64* %z.addr
  %t129 = bitcast i64 %t128 to i8*
  %t130 = add i8* %t127, %t129
  %t131 = getelementptr [4 x i8], [4 x i8]* @.str.16, i32 0, i32 0
  %t132 = add i8* %t130, %t131
  %t133 = load i64, i64* %w.addr
  %t134 = bitcast i64 %t133 to i8*
  %t135 = add i8* %t132, %t134
  %t136 = getelementptr [4 x i8], [4 x i8]* @.str.1, i32 0, i32 0
  %t137 = call i32 (i8*, ...) @printf(i8* %t136, i8* %t135)
  ret i32 0
}

