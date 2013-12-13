; ModuleID = 'loop8.simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [9 x i8] c"argc > 1\00", align 1
@.str1 = private unnamed_addr constant [8 x i8] c"loop8.c\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [29 x i8] c"int main(int, const char **)\00", align 1
@.str2 = private unnamed_addr constant [7 x i8] c"%d %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %a = alloca [10 x i32], align 16
  %b = alloca [10 x i32], align 16
  %c = alloca [10 x i32], align 16
  %0 = bitcast [10 x i32]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 40, i32 16, i1 false)
  %1 = bitcast [10 x i32]* %b to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 40, i32 16, i1 false)
  %2 = bitcast [10 x i32]* %c to i8*
  call void @llvm.memset.p0i8.i64(i8* %2, i8 0, i64 40, i32 16, i1 false)
  %cmp = icmp sgt i32 %argc, 1
  br i1 %cmp, label %cond.end, label %cond.false

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* getelementptr inbounds ([9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8]* @.str1, i64 0, i64 0), i32 10, i8* getelementptr inbounds ([29 x i8]* @__PRETTY_FUNCTION__.main, i64 0, i64 0)) #5
  unreachable

cond.end:                                         ; preds = %entry
  %arrayidx = getelementptr inbounds i8** %argv, i64 1
  %3 = load i8** %arrayidx, align 8
  %call = call i32 @atoi(i8* %3) #6
  %arrayidx1 = getelementptr inbounds [10 x i32]* %a, i64 0, i64 0
  store i32 %call, i32* %arrayidx1, align 16
  br label %for.cond

for.cond:                                         ; preds = %for.body, %cond.end
  %storemerge = phi i32 [ 1, %cond.end ], [ %inc, %for.body ]
  %cmp2 = icmp ult i32 %storemerge, 10
  br i1 %cmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %storemerge to i64
  %arrayidx3 = getelementptr inbounds [10 x i32]* %c, i64 0, i64 %idxprom
  %4 = load i32* %arrayidx3, align 4
  %idxprom4 = zext i32 %storemerge to i64
  %arrayidx5 = getelementptr inbounds [10 x i32]* %a, i64 0, i64 %idxprom4
  store i32 %4, i32* %arrayidx5, align 4
  %sub = add i32 %storemerge, -1
  %idxprom6 = zext i32 %sub to i64
  %arrayidx7 = getelementptr inbounds [10 x i32]* %a, i64 0, i64 %idxprom6
  %5 = load i32* %arrayidx7, align 4
  %add = add nsw i32 %5, 1
  %idxprom8 = zext i32 %storemerge to i64
  %arrayidx9 = getelementptr inbounds [10 x i32]* %b, i64 0, i64 %idxprom8
  store i32 %add, i32* %arrayidx9, align 4
  %inc = add i32 %storemerge, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %arrayidx10 = getelementptr inbounds [10 x i32]* %a, i64 0, i64 9
  %6 = load i32* %arrayidx10, align 4
  %arrayidx11 = getelementptr inbounds [10 x i32]* %b, i64 0, i64 9
  %7 = load i32* %arrayidx11, align 4
  %call12 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([7 x i8]* @.str2, i64 0, i64 0), i32 %6, i32 %7) #1
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
