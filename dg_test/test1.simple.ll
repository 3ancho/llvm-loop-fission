; ModuleID = 'test1.simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test(i32* %A) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %storemerge = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %storemerge, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %storemerge to i64
  %arrayidx = getelementptr inbounds i32* %A, i64 %idxprom
  %0 = load i32* %arrayidx, align 4
  %sub = add nsw i32 %0, -1
  %sub1 = add nsw i32 %storemerge, -1
  %idxprom2 = sext i32 %sub1 to i64
  %arrayidx3 = getelementptr inbounds i32* %A, i64 %idxprom2
  store i32 %sub, i32* %arrayidx3, align 4
  %add = add nsw i32 %0, 1
  %idxprom4 = sext i32 %storemerge to i64
  %arrayidx5 = getelementptr inbounds i32* %A, i64 %idxprom4
  store i32 %add, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %0, 2
  %add7 = add nsw i32 %storemerge, 1
  %idxprom8 = sext i32 %add7 to i64
  %arrayidx9 = getelementptr inbounds i32* %A, i64 %idxprom8
  store i32 %add6, i32* %arrayidx9, align 4
  %add10 = add nsw i32 %0, 4
  %add11 = add nsw i32 %storemerge, 2
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds i32* %A, i64 %idxprom12
  store i32 %add10, i32* %arrayidx13, align 4
  %inc = add nsw i32 %storemerge, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
