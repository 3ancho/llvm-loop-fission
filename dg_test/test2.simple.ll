; ModuleID = 'test2.simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test(i32* %A) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %storemerge = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %storemerge, 100
  br i1 %cmp, label %for.body, label %for.cond12

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %storemerge to i64
  %arrayidx = getelementptr inbounds i32* %A, i64 %idxprom
  %0 = load i32* %arrayidx, align 4
  %sub = add nsw i32 %0, -1
  %sub1 = add nsw i32 %storemerge, -1
  %idxprom2 = sext i32 %sub1 to i64
  %arrayidx3 = getelementptr inbounds i32* %A, i64 %idxprom2
  store i32 %sub, i32* %arrayidx3, align 4
  %add = add nsw i32 %0, 2
  %add4 = add nsw i32 %storemerge, 1
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds i32* %A, i64 %idxprom5
  store i32 %add, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %0, 4
  %add8 = add nsw i32 %storemerge, 2
  %idxprom9 = sext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i32* %A, i64 %idxprom9
  store i32 %add7, i32* %arrayidx10, align 4
  %inc = add nsw i32 %storemerge, 1
  br label %for.cond

for.cond12:                                       ; preds = %for.inc29, %for.cond
  %storemerge1 = phi i32 [ %inc30, %for.inc29 ], [ 1, %for.cond ]
  %cmp13 = icmp slt i32 %storemerge1, 10
  br i1 %cmp13, label %for.cond15, label %for.end31

for.cond15:                                       ; preds = %for.body17, %for.cond12
  %storemerge2 = phi i32 [ %inc27, %for.body17 ], [ 0, %for.cond12 ]
  %cmp16 = icmp slt i32 %storemerge2, 100
  br i1 %cmp16, label %for.body17, label %for.inc29

for.body17:                                       ; preds = %for.cond15
  %idxprom18 = sext i32 %storemerge2 to i64
  %arrayidx19 = getelementptr inbounds i32* %A, i64 %idxprom18
  store i32 5, i32* %arrayidx19, align 4
  %idxprom20 = sext i32 %storemerge2 to i64
  %arrayidx21 = getelementptr inbounds i32* %A, i64 %idxprom20
  %1 = load i32* %arrayidx21, align 4
  %sub22 = add nsw i32 %1, -1
  %add23 = add nsw i32 %storemerge2, 1
  %idxprom24 = sext i32 %add23 to i64
  %arrayidx25 = getelementptr inbounds i32* %A, i64 %idxprom24
  store i32 %sub22, i32* %arrayidx25, align 4
  %inc27 = add nsw i32 %storemerge2, 1
  br label %for.cond15

for.inc29:                                        ; preds = %for.cond15
  %inc30 = add nsw i32 %storemerge1, 1
  br label %for.cond12

for.end31:                                        ; preds = %for.cond12
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
