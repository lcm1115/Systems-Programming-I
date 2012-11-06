        .globl  cmp_course
cmp_course:
        enter   $0, $0
        xor     %eax, %eax

# Compare first char
        movb    8(%ebp), %bl
        movb    20(%ebp), %cl
        cmpb    %cl, %bl
        jg      greater
        jl      less

# Compare second char
        movb    12(%ebp), %bl
        movb    24(%ebp), %cl
        cmpb    %cl, %bl
        jg      greater
        jl      less

# Compare course ID short
        movw    16(%ebp), %bx
        movw    28(%ebp), %cx
        cmpw    %cx, %bx
        jg      greater
        jl      less

# Return 0
        jmp     done

# Return 1
greater:
        movl    $1, %eax
        jmp     done

# Return -1
less:
        movl    $-1, %eax

done:
        leave
        ret
