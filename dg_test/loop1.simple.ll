; ModuleID = 'loop1.simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [9 x i8] c"argc > 1\00", align 1
@.str1 = private unnamed_addr constant [8 x i8] c"loop1.c\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [29 x i8] c"int main(int, const char **)\00", align 1
@.str2 = private unnamed_addr constant [13 x i8] c"%d %d %d %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %a = alloca [100 x i32], align 16
  %b = alloca [100 x i32], align 16
  %c = alloca [100 x i32], align 16
  %d = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 400, i32 16, i1 false)
  %1 = bitcast [100 x i32]* %b to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 400, i32 16, i1 false)
  %2 = bitcast [100 x i32]* %c to i8*
  call void @llvm.memset.p0i8.i64(i8* %2, i8 0, i64 400, i32 16, i1 false)
  %3 = bitcast [100 x i32]* %d to i8*
  call void @llvm.memset.p0i8.i64(i8* %3, i8 0, i64 400, i32 16, i1 false)
  %cmp = icmp sgt i32 %argc, 1
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* getelementptr inbounds ([9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8]* @.str1, i64 0, i64 0), i32 11, i8* getelementptr inbounds ([29 x i8]* @__PRETTY_FUNCTION__.main, i64 0, i64 0)) #5
  unreachable

cond.end:                                         ; preds = %entry
  %arrayidx = getelementptr inbounds i8** %argv, i64 1
  %4 = load i8** %arrayidx, align 8
  %call = call i32 @atoi(i8* %4) #6
  %arrayidx1 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 0
  store i32 %call, i32* %arrayidx1, align 16
  %mul = shl nsw i32 %call, 1
  %arrayidx2 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 3
  store i32 %mul, i32* %arrayidx2, align 4
  %add = add nsw i32 %call, 1
  %arrayidx3 = getelementptr inbounds [100 x i32]* %c, i64 0, i64 1
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %cond.end
  %storemerge = phi i32 [ 2, %cond.end ], [ %inc, %for.body ]
  %cmp4 = icmp ult i32 %storemerge, 99
  br i1 %cmp4, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %mul5 = mul i32 %call, %storemerge
  %idxprom = zext i32 %storemerge to i64
  %arrayidx6 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 %idxprom
  store i32 %mul5, i32* %arrayidx6, align 4
  %sub = add i32 %storemerge, -2
  %idxprom7 = zext i32 %sub to i64
  %arrayidx8 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 %idxprom7
  %5 = load i32* %arrayidx8, align 4
  %add9 = add nsw i32 %5, %call
  %idxprom10 = zext i32 %storemerge to i64
  %arrayidx11 = getelementptr inbounds [100 x i32]* %b, i64 0, i64 %idxprom10
  store i32 %add9, i32* %arrayidx11, align 4
  %idxprom12 = zext i32 %storemerge to i64
  %arrayidx13 = getelementptr inbounds [100 x i32]* %b, i64 0, i64 %idxprom12
  %6 = load i32* %arrayidx13, align 4
  %add14 = add i32 %storemerge, 1
  %idxprom15 = zext i32 %add14 to i64
  %arrayidx16 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 %idxprom15
  %7 = load i32* %arrayidx16, align 4
  %add17 = add nsw i32 %6, %7
  %idxprom18 = zext i32 %storemerge to i64
  %arrayidx19 = getelementptr inbounds [100 x i32]* %c, i64 0, i64 %idxprom18
  store i32 %add17, i32* %arrayidx19, align 4
  %sub20 = add i32 %storemerge, -1
  %idxprom21 = zext i32 %sub20 to i64
  %arrayidx22 = getelementptr inbounds [100 x i32]* %c, i64 0, i64 %idxprom21
  %8 = load i32* %arrayidx22, align 4
  %add23 = add nsw i32 %8, %call
  %add24 = add i32 %add23, %storemerge
  %idxprom25 = zext i32 %storemerge to i64
  %arrayidx26 = getelementptr inbounds [100 x i32]* %d, i64 0, i64 %idxprom25
  store i32 %add24, i32* %arrayidx26, align 4
  %inc = add i32 %storemerge, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx27 = getelementptr inbounds [100 x i32]* %a, i64 0, i64 98
  %9 = load i32* %arrayidx27, align 8
  %arrayidx28 = getelementptr inbounds [100 x i32]* %b, i64 0, i64 99
  %10 = load i32* %arrayidx28, align 4
  %arrayidx29 = getelementptr inbounds [100 x i32]* %c, i64 0, i64 98
  %11 = load i32* %arrayidx29, align 8
  %arrayidx30 = getelementptr inbounds [100 x i32]* %d, i64 0, i64 98
  %12 = load i32* %arrayidx30, align 8
  %call31 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([13 x i8]* @.str2, i64 0, i64 0), i32 %9, i32 %10, i32 %11, i32 %12) #1
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

; Function Attrs: noreturn nounwind
declare void @__assert_fail(i8*, i8*, i32, i8*) #2

; Function Attrs: nounwind readonly
declare i32 @atoi(i8*) #3

declare i32 @printf(i8*, ...) #4

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { noreturn nounwind }
attributes #6 = { nounwind readonly }
