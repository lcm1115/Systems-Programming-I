        .globl  cmp_student
cmp_student:
        enter   $0, $0

# Load string addresses
        movl    8(%ebp), %edx
        movl    12(%ebp), %ecx
        movl    8(%edx), %edx
        movl    8(%ecx), %ecx

# Call strcmp to compare names
        pushl   %ecx
        pushl   %edx
        call    strcmp
        popl    %edx
        popl    %edx

# If names are different, return result from strcmp
        cmpl    $0, %eax
        jne     done

# Otherwise compare sid int
        movl    8(%ebp), %edx
        movl    12(%ebp), %ecx
        movl    12(%edx), %edx
        movl    12(%ecx), %ecx

        cmpl    %ecx, %edx
        jg      greater
        jl      less

# Return 0 (should realistically never happen)
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
