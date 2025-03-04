; RUN: opt < %s -basicaa -functionattrs -S | grep readnone

@s = external constant i8		; <i8*> [#uses=1]

define i8 @f() {
	%tmp = load i8, i8* @s		; <i8> [#uses=1]
	ret i8 %tmp
}
