; ModuleID = 'test.simple.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @test(i32* %A) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv3 = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32* %A, i64 %indvars.iv3
  %0 = load i32* %arrayidx, align 4
  %sub = add nsw i32 %0, -1
  %1 = add nsw i64 %indvars.iv3, -1
  %arrayidx3 = getelementptr inbounds i32* %A, i64 %1
  store i32 %sub, i32* %arrayidx3, align 4
  %add = add nsw i32 %0, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv3, 1
  %arrayidx6 = getelementptr inbounds i32* %A, i64 %indvars.iv.next
  store i32 %add, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %0, 2
  %2 = add nsw i64 %indvars.iv3, 2
  %arrayidx10 = getelementptr inbounds i32* %A, i64 %2
  store i32 %add7, i32* %arrayidx10, align 4
  %exitcond = icmp ne i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5 (trunk 195566)"}
