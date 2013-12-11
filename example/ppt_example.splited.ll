; ModuleID = 'example/ppt_example.splited.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@main.a = private unnamed_addr constant [5 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4], align 16
@.str = private unnamed_addr constant [48 x i8] c"a[%d] = %d, b[%d] = %d, c[%d] = %d, d[%d] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %a = alloca [5 x i32], align 16
  %b = alloca [5 x i32], align 16
  %c = alloca [5 x i32], align 16
  %d = alloca [5 x i32], align 16
  %0 = bitcast [5 x i32]* %a to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %0, i8* bitcast ([5 x i32]* @main.a to i8*), i64 20, i32 16, i1 false)
  %1 = bitcast [5 x i32]* %b to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 20, i32 16, i1 false)
  %2 = bitcast [5 x i32]* %c to i8*
  call void @llvm.memset.p0i8.i64(i8* %2, i8 0, i64 20, i32 16, i1 false)
  %3 = bitcast [5 x i32]* %d to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 20, i32 16, i1 false)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %indvars.iv1 = phi i64 [ %indvars.iv.next2, %for.inc ], [ 1, %entry ]
  %lftr.wideiv5 = trunc i64 %indvars.iv1 to i32
  %exitcond6 = icmp ne i32 %lftr.wideiv5, 5
  br i1 %exitcond6, label %for.body, label %for.cond.copied

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv1
  %4 = load i32* %arrayidx, align 4
  %5 = add nsw i64 %indvars.iv1, -1
  %arrayidx2 = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %5
  %6 = load i32* %arrayidx2, align 4
  %add = add nsw i32 %4, %6
  %arrayidx4 = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv1
  store i32 %add, i32* %arrayidx4, align 4
  %7 = add nsw i64 %indvars.iv1, -1
  %arrayidx7 = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %7
  %8 = load i32* %arrayidx7, align 4
  %add8 = add nsw i32 %8, 5
  %arrayidx10 = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv1
  store i32 %add8, i32* %arrayidx10, align 4
  %arrayidx12 = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv1
  %9 = load i32* %arrayidx12, align 4
  %mul = shl nsw i32 %9, 1
  %arrayidx14 = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv1
  store i32 %mul, i32* %arrayidx14, align 4
  %arrayidx16 = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv1
  %10 = load i32* %arrayidx16, align 4
  %add17 = add nsw i32 %10, 1
  %arrayidx19 = getelementptr inbounds [5 x i32]* %d, i64 0, i64 %indvars.iv1
  store i32 %add17, i32* %arrayidx19, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next2 = add i64 %indvars.iv1, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond.copied
  br label %for.cond20

for.cond20:                                       ; preds = %for.inc31, %for.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc31 ], [ 0, %for.end ]
  %lftr.wideiv = trunc i64 %indvars.iv to i32
  %exitcond = icmp ne i32 %lftr.wideiv, 5
  br i1 %exitcond, label %for.body22, label %for.cond20.copied

for.body22:                                       ; preds = %for.cond20
  %arrayidx24 = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv
  %11 = load i32* %arrayidx24, align 4
  %arrayidx26 = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv
  %12 = load i32* %arrayidx26, align 4
  %arrayidx28 = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv
  %13 = load i32* %arrayidx28, align 4
  %arrayidx30 = getelementptr inbounds [5 x i32]* %d, i64 0, i64 %indvars.iv
  %14 = load i32* %arrayidx30, align 4
  %15 = trunc i64 %indvars.iv to i32
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([48 x i8]* @.str, i64 0, i64 0), i32 %15, i32 %11, i32 %15, i32 %12, i32 %15, i32 %13, i32 %15, i32 %14) #1
  br label %for.inc31

for.inc31:                                        ; preds = %for.body22
  %indvars.iv.next = add i64 %indvars.iv, 1
  br label %for.cond20

for.end33:                                        ; preds = %for.cond20.copied
  ret i32 0

for.cond20.copied:                                ; preds = %for.inc31.copied, %for.cond20
  %indvars.iv.copied = phi i64 [ %indvars.iv.next.copied, %for.inc31.copied ], [ 0, %for.cond20 ]
  %lftr.wideiv.copied = trunc i64 %indvars.iv.copied to i32
  %exitcond.copied = icmp ne i32 %lftr.wideiv.copied, 5
  br i1 %exitcond.copied, label %for.body22.copied, label %for.end33

for.body22.copied:                                ; preds = %for.cond20.copied
  %arrayidx24.copied = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv.copied
  %16 = load i32* %arrayidx24.copied, align 4
  %arrayidx26.copied = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv.copied
  %17 = load i32* %arrayidx26.copied, align 4
  %arrayidx28.copied = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv.copied
  %18 = load i32* %arrayidx28.copied, align 4
  %arrayidx30.copied = getelementptr inbounds [5 x i32]* %d, i64 0, i64 %indvars.iv.copied
  %19 = load i32* %arrayidx30.copied, align 4
  %20 = trunc i64 %indvars.iv.copied to i32
  %call.copied = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([48 x i8]* @.str, i64 0, i64 0), i32 %20, i32 %16, i32 %20, i32 %17, i32 %20, i32 %18, i32 %20, i32 %19) #1
  br label %for.inc31.copied

for.inc31.copied:                                 ; preds = %for.body22.copied
  %indvars.iv.next.copied = add i64 %indvars.iv.copied, 1
  br label %for.cond20.copied

for.cond.copied:                                  ; preds = %for.inc.copied, %for.cond
  %indvars.iv1.copied = phi i64 [ %indvars.iv.next2.copied, %for.inc.copied ], [ 1, %for.cond ]
  %lftr.wideiv5.copied = trunc i64 %indvars.iv1.copied to i32
  %exitcond6.copied = icmp ne i32 %lftr.wideiv5.copied, 5
  br i1 %exitcond6.copied, label %for.body.copied, label %for.end

for.body.copied:                                  ; preds = %for.cond.copied
  %arrayidx.copied = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv1.copied
  %21 = load i32* %arrayidx.copied, align 4
  %22 = add nsw i64 %indvars.iv1.copied, -1
  %arrayidx2.copied = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %22
  %23 = load i32* %arrayidx2.copied, align 4
  %add.copied = add nsw i32 %21, %23
  %arrayidx4.copied = getelementptr inbounds [5 x i32]* %a, i64 0, i64 %indvars.iv1.copied
  store i32 %add.copied, i32* %arrayidx4.copied, align 4
  %24 = add nsw i64 %indvars.iv1.copied, -1
  %arrayidx7.copied = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %24
  %25 = load i32* %arrayidx7.copied, align 4
  %add8.copied = add nsw i32 %25, 5
  %arrayidx10.copied = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv1.copied
  store i32 %add8.copied, i32* %arrayidx10.copied, align 4
  %arrayidx12.copied = getelementptr inbounds [5 x i32]* %b, i64 0, i64 %indvars.iv1.copied
  %26 = load i32* %arrayidx12.copied, align 4
  %mul.copied = shl nsw i32 %26, 1
  %arrayidx14.copied = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv1.copied
  store i32 %mul.copied, i32* %arrayidx14.copied, align 4
  %arrayidx16.copied = getelementptr inbounds [5 x i32]* %c, i64 0, i64 %indvars.iv1.copied
  %27 = load i32* %arrayidx16.copied, align 4
  %add17.copied = add nsw i32 %27, 1
  %arrayidx19.copied = getelementptr inbounds [5 x i32]* %d, i64 0, i64 %indvars.iv1.copied
  store i32 %add17.copied, i32* %arrayidx19.copied, align 4
  br label %for.inc.copied

for.inc.copied:                                   ; preds = %for.body.copied
  %indvars.iv.next2.copied = add i64 %indvars.iv1.copied, 1
  br label %for.cond.copied
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
