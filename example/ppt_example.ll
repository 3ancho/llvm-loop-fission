; ModuleID = 'example/ppt_example.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@main.a = private unnamed_addr constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16
@.str = private unnamed_addr constant [48 x i8] c"a[%d] = %d, b[%d] = %d, c[%d] = %d, d[%d] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [5 x i32], align 16
  %b = alloca [5 x i32], align 16
  %c = alloca [5 x i32], align 16
  %d = alloca [5 x i32], align 16
  %i = alloca i32, align 4
  store i32 0, i32* %retval
  %0 = bitcast [5 x i32]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %0, i8* bitcast ([5 x i32]* @main.a to i8*), i64 20, i32 16, i1 false)
  %1 = bitcast [5 x i32]* %b to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 20, i32 16, i1 false)
  %2 = bitcast [5 x i32]* %c to i8*
  call void @llvm.memset.p0i8.i64(i8* %2, i8 0, i64 20, i32 16, i1 false)
  %3 = bitcast [5 x i32]* %d to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 20, i32 16, i1 false)
  store i32 1, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %4 = load i32* %i, align 4
  %cmp = icmp slt i32 %4, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %5 = load i32* %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [5 x i32]* %a, i32 0, i64 %idxprom
  %6 = load i32* %arrayidx, align 4
  %7 = load i32* %i, align 4
  %sub = sub nsw i32 %7, 1
  %idxprom1 = sext i32 %sub to i64
  %arrayidx2 = getelementptr inbounds [5 x i32]* %b, i32 0, i64 %idxprom1
  %8 = load i32* %arrayidx2, align 4
  %add = add nsw i32 %6, %8
  %9 = load i32* %i, align 4
  %idxprom3 = sext i32 %9 to i64
  %arrayidx4 = getelementptr inbounds [5 x i32]* %a, i32 0, i64 %idxprom3
  store i32 %add, i32* %arrayidx4, align 4
  %10 = load i32* %i, align 4
  %sub5 = sub nsw i32 %10, 1
  %idxprom6 = sext i32 %sub5 to i64
  %arrayidx7 = getelementptr inbounds [5 x i32]* %c, i32 0, i64 %idxprom6
  %11 = load i32* %arrayidx7, align 4
  %add8 = add nsw i32 %11, 5
  %12 = load i32* %i, align 4
  %idxprom9 = sext i32 %12 to i64
  %arrayidx10 = getelementptr inbounds [5 x i32]* %b, i32 0, i64 %idxprom9
  store i32 %add8, i32* %arrayidx10, align 4
  %13 = load i32* %i, align 4
  %idxprom11 = sext i32 %13 to i64
  %arrayidx12 = getelementptr inbounds [5 x i32]* %b, i32 0, i64 %idxprom11
  %14 = load i32* %arrayidx12, align 4
  %mul = mul nsw i32 %14, 2
  %15 = load i32* %i, align 4
  %idxprom13 = sext i32 %15 to i64
  %arrayidx14 = getelementptr inbounds [5 x i32]* %c, i32 0, i64 %idxprom13
  store i32 %mul, i32* %arrayidx14, align 4
  %16 = load i32* %i, align 4
  %idxprom15 = sext i32 %16 to i64
  %arrayidx16 = getelementptr inbounds [5 x i32]* %c, i32 0, i64 %idxprom15
  %17 = load i32* %arrayidx16, align 4
  %add17 = add nsw i32 %17, 1
  %18 = load i32* %i, align 4
  %idxprom18 = sext i32 %18 to i64
  %arrayidx19 = getelementptr inbounds [5 x i32]* %d, i32 0, i64 %idxprom18
  store i32 %add17, i32* %arrayidx19, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %19 = load i32* %i, align 4
  %inc = add nsw i32 %19, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i, align 4
  br label %for.cond20

for.cond20:                                       ; preds = %for.inc31, %for.end
  %20 = load i32* %i, align 4
  %cmp21 = icmp slt i32 %20, 5
  br i1 %cmp21, label %for.body22, label %for.end33

for.body22:                                       ; preds = %for.cond20
  %21 = load i32* %i, align 4
  %22 = load i32* %i, align 4
  %idxprom23 = sext i32 %22 to i64
  %arrayidx24 = getelementptr inbounds [5 x i32]* %a, i32 0, i64 %idxprom23
  %23 = load i32* %arrayidx24, align 4
  %24 = load i32* %i, align 4
  %25 = load i32* %i, align 4
  %idxprom25 = sext i32 %25 to i64
  %arrayidx26 = getelementptr inbounds [5 x i32]* %b, i32 0, i64 %idxprom25
  %26 = load i32* %arrayidx26, align 4
  %27 = load i32* %i, align 4
  %28 = load i32* %i, align 4
  %idxprom27 = sext i32 %28 to i64
  %arrayidx28 = getelementptr inbounds [5 x i32]* %c, i32 0, i64 %idxprom27
  %29 = load i32* %arrayidx28, align 4
  %30 = load i32* %i, align 4
  %31 = load i32* %i, align 4
  %idxprom29 = sext i32 %31 to i64
  %arrayidx30 = getelementptr inbounds [5 x i32]* %d, i32 0, i64 %idxprom29
  %32 = load i32* %arrayidx30, align 4
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([48 x i8]* @.str, i32 0, i32 0), i32 %21, i32 %23, i32 %24, i32 %26, i32 %27, i32 %29, i32 %30, i32 %32)
  br label %for.inc31

for.inc31:                                        ; preds = %for.body22
  %33 = load i32* %i, align 4
  %inc32 = add nsw i32 %33, 1
  store i32 %inc32, i32* %i, align 4
  br label %for.cond20

for.end33:                                        ; preds = %for.cond20
  %34 = load i32* %retval
  ret i32 %34
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
