; RUN: opt < %s -gvn -S | not grep tmp2
; PR2213

define i32* @f(i8* %x) {
entry:
        %tmp = call i8* @m( i32 12 )            ; <i8*> [#uses=2]
        %tmp1 = bitcast i8* %tmp to i32*                ; <i32*> [#uses=0]
        %tmp2 = bitcast i8* %tmp to i32*                ; <i32*> [#uses=0]
        ret i32* %tmp2
}

declare i8* @m(i32)
