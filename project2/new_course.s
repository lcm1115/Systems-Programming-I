        .globl  new_course
new_course:
        enter   $0, $0
        pushl   $16
        call    allocate
        popl    %edx

        cmpl    $0, %eax
        je      done

        movl    $0, (%eax)
        movl    $0, 4(%eax)
        movl    $0, 13(%eax)

        movb    8(%ebp), %cl
        movb    %cl, 10(%eax)

        movb    12(%ebp), %cl
        movb    %cl, 11(%eax)

        movw    16(%ebp), %cx
        movw    %cx, 8(%eax)

        movb    20(%ebp), %cl
        movb    %cl, 12(%eax)

done:
        leave
        ret
